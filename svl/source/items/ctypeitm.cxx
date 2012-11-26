/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
#include <com/sun/star/uno/Any.hxx>

#include <unotools/intlwrapper.hxx>
#include <tools/stream.hxx>
#include <svl/ctypeitm.hxx>

//============================================================================
// The following defines are copied from chaos/source/items/cstritem.cxx:
#define CNTSTRINGITEM_STREAM_MAGIC   ( (sal_uInt32)0xfefefefe )
#define CNTSTRINGITEM_STREAM_SEEKREL (-( (long)( sizeof( sal_uInt32 ) ) ) )

//============================================================================
//
// class CntContentTypeItem Implementation.
//
//============================================================================

#define CONTENT_TYPE_NOT_INIT ( (INetContentType)-1 )

//----------------------------------------------------------------------------
CntContentTypeItem::CntContentTypeItem()
: CntUnencodedStringItem(),
  _eType( CONTENT_TYPE_NOT_INIT )
{
}

//----------------------------------------------------------------------------
CntContentTypeItem::CntContentTypeItem( sal_uInt16 which, const XubString& rType )
: CntUnencodedStringItem( which, rType ),
  _eType( CONTENT_TYPE_NOT_INIT )
{
}

//----------------------------------------------------------------------------
CntContentTypeItem::CntContentTypeItem( sal_uInt16 which,
                                        const INetContentType eType )
: CntUnencodedStringItem( which, INetContentTypes::GetContentType( eType ) ),
  _eType( eType )
{
}

//----------------------------------------------------------------------------
CntContentTypeItem::CntContentTypeItem( const CntContentTypeItem& rOrig )
: CntUnencodedStringItem( rOrig ),
  _eType( rOrig._eType ),
  _aPresentation( rOrig._aPresentation )
{
}

//============================================================================
// virtual
sal_uInt16 CntContentTypeItem::GetVersion(sal_uInt16) const
{
    return 1; // because it uses SfxPoolItem::read/writeUnicodeString()
}

//----------------------------------------------------------------------------
// virtual
SfxPoolItem* CntContentTypeItem::Create( SvStream& rStream,
                                         sal_uInt16 nItemVersion ) const
{
    // CntContentTypeItem used to be derived from CntStringItem, so take that
    // into account:
    UniString aValue;
    readUnicodeString(rStream, aValue, nItemVersion >= 1);
    sal_uInt32 nMagic = 0;
    rStream >> nMagic;
    if (nMagic == CNTSTRINGITEM_STREAM_MAGIC)
    {
        sal_Bool bEncrypted = sal_False;
        rStream >> bEncrypted;
        DBG_ASSERT(!bEncrypted,
                   "CntContentTypeItem::Create() reads encrypted data");
    }
    else
        rStream.SeekRel(CNTSTRINGITEM_STREAM_SEEKREL);

    return new CntContentTypeItem(Which(), aValue);
}

//----------------------------------------------------------------------------
// virtual
SvStream & CntContentTypeItem::Store(SvStream & rStream, sal_uInt16) const
{
    // CntContentTypeItem used to be derived from CntStringItem, so take that
    // into account:
    writeUnicodeString(rStream, GetValue());
    rStream << CNTSTRINGITEM_STREAM_MAGIC << sal_Bool(sal_False);
    return rStream;
}

//----------------------------------------------------------------------------
// virtual
int CntContentTypeItem::operator==( const SfxPoolItem& rOrig ) const
{
    const CntContentTypeItem& rOther = (const CntContentTypeItem&)rOrig;

    if ( ( _eType != CONTENT_TYPE_NOT_INIT ) &&
         ( rOther._eType != CONTENT_TYPE_NOT_INIT ) )
        return _eType == rOther._eType;
    else
        return CntUnencodedStringItem::operator==( rOther );
}

//----------------------------------------------------------------------------
// virtual
SfxPoolItem* CntContentTypeItem::Clone( SfxItemPool* /* pPool */ ) const
{
    return new CntContentTypeItem( *this );
}

//----------------------------------------------------------------------------
void CntContentTypeItem::SetValue( const XubString& rNewVal )
{
    // De-initialize enum type and presentation.
    _eType = CONTENT_TYPE_NOT_INIT;
    _aPresentation.Erase();

    CntUnencodedStringItem::SetValue( rNewVal );
}

//----------------------------------------------------------------------------
void CntContentTypeItem::SetPresentation( const XubString& rNewVal )
{
    _aPresentation = rNewVal;
}

//----------------------------------------------------------------------------
int CntContentTypeItem::Compare( const SfxPoolItem &rWith, const IntlWrapper& rIntlWrapper ) const
{
    String aOwnText, aWithText;
    GetPresentation( SFX_ITEM_PRESENTATION_NAMELESS,
                     SFX_MAPUNIT_APPFONT, SFX_MAPUNIT_APPFONT, aOwnText, &rIntlWrapper );
    rWith.GetPresentation( SFX_ITEM_PRESENTATION_NAMELESS,
                           SFX_MAPUNIT_APPFONT, SFX_MAPUNIT_APPFONT, aWithText, &rIntlWrapper );
    return rIntlWrapper.getCollator()->compareString( aOwnText, aWithText );
}

//----------------------------------------------------------------------------
SfxItemPresentation CntContentTypeItem::GetPresentation(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreMetric,
    SfxMapUnit          ePresMetric,
    XubString         & rText,
    const IntlWrapper * pIntlWrapper) const
{
    if (_aPresentation.Len() == 0)
    {
        DBG_ASSERT(pIntlWrapper,
                   "CntContentTypeItem::GetPresentation(): No IntlWrapper");
        if (pIntlWrapper)
            SAL_CONST_CAST(CntContentTypeItem *, this)->_aPresentation
             = INetContentTypes::GetPresentation(GetEnumValue(),
                                                 pIntlWrapper->
                                                  getLocale());
    }
    if (_aPresentation.Len() > 0)
    {
        rText = _aPresentation;
        return SFX_ITEM_PRESENTATION_COMPLETE;
    }
    else
        return CntUnencodedStringItem::GetPresentation(ePres, eCoreMetric,
                                                       ePresMetric, rText,
                                                       pIntlWrapper);
}

//----------------------------------------------------------------------------
INetContentType CntContentTypeItem::GetEnumValue() const
{
    if ( _eType == CONTENT_TYPE_NOT_INIT )
    {
        // Not yet initialized... Get enum value for string content type.

        CntContentTypeItem* pVarThis = SAL_CONST_CAST( CntContentTypeItem*, this );

        pVarThis->_eType = INetContentTypes::GetContentType( GetValue() );
    }

    return _eType;
}

//----------------------------------------------------------------------------
void CntContentTypeItem::SetValue( const INetContentType eType )
{
    SetValue( INetContentTypes::GetContentType( eType ) );

    // Note: SetValue( const String& ....) resets _eType. Set new enum value
    //       after(!) calling it.
    _eType = eType;
}

//----------------------------------------------------------------------------
// virtual
sal_Bool CntContentTypeItem::QueryValue( com::sun::star::uno::Any& rVal,sal_uInt8 ) const
{
    rVal <<= rtl::OUString(GetValue());
    return true;
}

//----------------------------------------------------------------------------
// virtual
sal_Bool CntContentTypeItem::PutValue( const com::sun::star::uno::Any& rVal,sal_uInt8 )
{
    rtl::OUString aValue;
    if ( rVal >>= aValue )
    {
        // SetValue with an empty string resets the item; so call that
        // function when PutValue is called with an empty string
        if (aValue.getLength() == 0)
            SetValue(aValue);
        else
            SetValue(
                INetContentTypes::RegisterContentType(aValue, UniString()));
        return true;
    }

    DBG_ERROR( "CntContentTypeItem::PutValue - Wrong type!" );
    return false;
}
