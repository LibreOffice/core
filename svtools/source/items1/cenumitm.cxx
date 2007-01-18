/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cenumitm.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-18 14:13:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _SVTOOLS_CENUMITM_HXX
#include <cenumitm.hxx>
#endif

#ifndef _SFX_WHASSERT_HXX
#include <whassert.hxx>
#endif

#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

//============================================================================
//
//  class SfxEnumItemInterface
//
//============================================================================

DBG_NAME(SfxEnumItemInterface)

//============================================================================
TYPEINIT1(SfxEnumItemInterface, SfxPoolItem)

//============================================================================
// virtual
int SfxEnumItemInterface::operator ==(const SfxPoolItem & rItem) const
{
    SFX_ASSERT(SfxPoolItem::operator ==(rItem), Which(), "unequal type");
    return GetEnumValue()
               == static_cast< const SfxEnumItemInterface * >(&rItem)->
                      GetEnumValue();
}

//============================================================================
// virtual
SfxItemPresentation
SfxEnumItemInterface::GetPresentation(SfxItemPresentation, SfxMapUnit,
                                      SfxMapUnit, XubString & rText,
                                      const IntlWrapper *) const
{
    rText = XubString::CreateFromInt32(GetEnumValue());
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
BOOL SfxEnumItemInterface::QueryValue(com::sun::star::uno::Any& rVal, BYTE)
    const
{
    rVal <<= sal_Int32(GetEnumValue());
    return true;
}

//============================================================================
// virtual
BOOL SfxEnumItemInterface::PutValue(const com::sun::star::uno::Any& rVal,
                                    BYTE)
{
    sal_Int32 nTheValue = 0;

    if ( ::cppu::enum2int( nTheValue, rVal ) )
    {
        SetEnumValue(USHORT(nTheValue));
        return true;
    }
    DBG_ERROR("SfxEnumItemInterface::PutValue(): Wrong type");
    return false;
}

//============================================================================
XubString SfxEnumItemInterface::GetValueTextByPos(USHORT) const
{
    DBG_WARNING("SfxEnumItemInterface::GetValueTextByPos(): Pure virtual");
    return XubString();
}

//============================================================================
// virtual
USHORT SfxEnumItemInterface::GetValueByPos(USHORT nPos) const
{
    return nPos;
}

//============================================================================
// virtual
USHORT SfxEnumItemInterface::GetPosByValue(USHORT nValue) const
{
    USHORT nCount = GetValueCount();
    for (USHORT i = 0; i < nCount; ++i)
        if (GetValueByPos(i) == nValue)
            return i;
    return USHRT_MAX;
}

BOOL SfxEnumItemInterface::IsEnabled(USHORT) const
{
    return TRUE;
}

//============================================================================
// virtual
int SfxEnumItemInterface::HasBoolValue() const
{
    return false;
}

//============================================================================
// virtual
BOOL SfxEnumItemInterface::GetBoolValue() const
{
    return false;
}

//============================================================================
// virtual
void SfxEnumItemInterface::SetBoolValue(BOOL)
{}

//============================================================================
//
//  class CntEnumItem
//
//============================================================================

DBG_NAME(CntEnumItem)

//============================================================================
CntEnumItem::CntEnumItem(USHORT which, SvStream & rStream):
    SfxEnumItemInterface(which)
{
    m_nValue = 0;
    rStream >> m_nValue;
}

//============================================================================
TYPEINIT1(CntEnumItem, SfxEnumItemInterface)

//============================================================================
// virtual
SvStream & CntEnumItem::Store(SvStream & rStream, USHORT) const
{
    rStream << m_nValue;
    return rStream;
}

//============================================================================
// virtual
USHORT CntEnumItem::GetEnumValue() const
{
    return GetValue();
}

//============================================================================
// virtual
void CntEnumItem::SetEnumValue(USHORT nTheValue)
{
    SetValue(nTheValue);
}

//============================================================================
//
//  class CntBoolItem
//
//============================================================================

DBG_NAME(CntBoolItem)

//============================================================================
TYPEINIT1_AUTOFACTORY(CntBoolItem, SfxPoolItem)

//============================================================================
CntBoolItem::CntBoolItem(USHORT which, SvStream & rStream):
    SfxPoolItem(which)
{
    m_bValue = false;
    rStream >> m_bValue;
}

//============================================================================
// virtual
int CntBoolItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(rItem.ISA(CntBoolItem),
               "CntBoolItem::operator ==(): Bad type");
    return m_bValue == static_cast< CntBoolItem const * >(&rItem)->m_bValue;
}

//============================================================================
// virtual
int CntBoolItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_ASSERT(rWith.ISA(CntBoolItem), "CntBoolItem::Compare(): Bad type");
    return m_bValue == static_cast< CntBoolItem const * >(&rWith)->m_bValue ?
               0 : m_bValue ? -1 : 1;
}

//============================================================================
// virtual
SfxItemPresentation CntBoolItem::GetPresentation(SfxItemPresentation,
                                                 SfxMapUnit, SfxMapUnit,
                                                 UniString & rText,
                                                 const IntlWrapper *) const
{
    rText = GetValueTextByVal(m_bValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
BOOL CntBoolItem::QueryValue(com::sun::star::uno::Any& rVal, BYTE) const
{
    rVal <<= sal_Bool(m_bValue);
    return true;
}

//============================================================================
// virtual
BOOL CntBoolItem::PutValue(const com::sun::star::uno::Any& rVal, BYTE)
{
    sal_Bool bTheValue = sal_Bool();
    if (rVal >>= bTheValue)
    {
        m_bValue = bTheValue;
        return true;
    }
    DBG_ERROR("CntBoolItem::PutValue(): Wrong type");
    return false;
}

//============================================================================
// virtual
SfxPoolItem * CntBoolItem::Create(SvStream & rStream, USHORT) const
{
    return new CntBoolItem(Which(), rStream);
}

//============================================================================
// virtual
SvStream & CntBoolItem::Store(SvStream & rStream, USHORT) const
{
    rStream << m_bValue;
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * CntBoolItem::Clone(SfxItemPool *) const
{
    return new CntBoolItem(*this);
}

//============================================================================
// virtual
USHORT CntBoolItem::GetValueCount() const
{
    return 2;
}

//============================================================================
// virtual
UniString CntBoolItem::GetValueTextByVal(BOOL bTheValue) const
{
    return
        bTheValue ?
            UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("TRUE")) :
            UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("FALSE"));
}

