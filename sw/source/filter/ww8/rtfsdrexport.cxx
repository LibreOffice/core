/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Miklos Vajna.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "rtfsdrexport.hxx"
#include "rtfexport.hxx"
#include "writerhelper.hxx"

#include <com/sun/star/i18n/ScriptType.hdl>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <svl/itemiter.hxx>
#include <svtools/rtfkeywd.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/svdotext.hxx>
#include <tools/stream.hxx>
#include <breakit.hxx>

using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringBuffer;
using namespace sw::util;

/// Implementation of an empty stream that silently succeeds, but does nothing.
///
/// In fact, this is a hack.  The right solution is to abstract EscherEx to be
/// able to work without SvStream; but at the moment it is better to live with
/// this I guess.
class SvNullStream : public SvStream
{
protected:
    virtual sal_Size GetData( void* pData, sal_Size nSize ) { memset( pData, 0, nSize ); return nSize; }
    virtual sal_Size PutData( const void*, sal_Size nSize ) { return nSize; }
    virtual sal_Size SeekPos( sal_Size nPos ) { return nPos; }
    virtual void SetSize( sal_Size ) {}
    virtual void FlushData() {}

public:
    SvNullStream() : SvStream() {}
    virtual ~SvNullStream() {}
};

RtfSdrExport::RtfSdrExport( RtfExport &rExport )
    : EscherEx( EscherExGlobalRef( new EscherExGlobal ), *( new SvNullStream )),
      m_rExport( rExport ),
      m_rAttrOutput( (RtfAttributeOutput&)m_rExport.AttrOutput() ),
      m_nShapeType( ESCHER_ShpInst_Nil ),
      m_pShapeStyle( new OStringBuffer( 200 ) ),
      m_pShapeTypeWritten( new bool[ ESCHER_ShpInst_COUNT ] )
{
    mnGroupLevel = 1;
    memset( m_pShapeTypeWritten, 0, ESCHER_ShpInst_COUNT * sizeof( bool ) );
}

RtfSdrExport::~RtfSdrExport()
{
    delete mpOutStrm, mpOutStrm = NULL;
    delete m_pShapeStyle, m_pShapeStyle = NULL;
    delete[] m_pShapeTypeWritten, m_pShapeTypeWritten = NULL;
}

void RtfSdrExport::OpenContainer( sal_uInt16 nEscherContainer, int nRecInstance )
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    EscherEx::OpenContainer( nEscherContainer, nRecInstance );

    if ( nEscherContainer == ESCHER_SpContainer )
    {
        m_nShapeType = ESCHER_ShpInst_Nil;
        if ( m_pShapeStyle->getLength() )
            m_pShapeStyle->makeStringAndClear();
        m_pShapeStyle->ensureCapacity( 200 );
        m_aShapeProps.clear();
    }
}

void RtfSdrExport::CloseContainer()
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    if ( mRecTypes.back() == ESCHER_SpContainer )
    {
        // write the shape now when we have all the info
        sal_Int32 nShapeElement = StartShape();
        EndShape( nShapeElement );

        // cleanup
        m_nShapeType = ESCHER_ShpInst_Nil;
    }

    EscherEx::CloseContainer();
}

sal_uInt32 RtfSdrExport::EnterGroup( const String& /*rShapeName*/, const Rectangle* /*pRect*/ )
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    return GenerateShapeId();
}

void RtfSdrExport::LeaveGroup()
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    /* noop */
}

void RtfSdrExport::AddShape( sal_uInt32 nShapeType, sal_uInt32 nShapeFlags, sal_uInt32 /*nShapeId*/ )
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    m_nShapeType = nShapeType;
    m_nShapeFlags = nShapeFlags;
}

inline sal_uInt16 impl_GetUInt16( const sal_uInt8* &pVal )
{
    sal_uInt16 nRet = *pVal++;
    nRet += ( *pVal++ ) << 8;
    return nRet;
}

inline sal_Int32 impl_GetPointComponent( const sal_uInt8* &pVal, sal_uInt16 nPointSize )
{
    sal_Int32 nRet = 0;
    if ( ( nPointSize == 0xfff0 ) || ( nPointSize == 4 ) )
    {
        sal_uInt16 nUnsigned = *pVal++;
        nUnsigned += ( *pVal++ ) << 8;

        nRet = sal_Int16( nUnsigned );
    }
    else if ( nPointSize == 8 )
    {
        sal_uInt32 nUnsigned = *pVal++;
        nUnsigned += ( *pVal++ ) << 8;
        nUnsigned += ( *pVal++ ) << 16;
        nUnsigned += ( *pVal++ ) << 24;

        nRet = nUnsigned;
    }

    return nRet;
}

void RtfSdrExport::Commit( EscherPropertyContainer& rProps, const Rectangle& rRect )
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    if ( m_nShapeType == ESCHER_ShpInst_Nil )
        return;

    if ( m_nShapeType == ESCHER_ShpInst_Line )
        AddLineDimensions( rRect );
    else
        AddRectangleDimensions( *m_pShapeStyle, rRect );

    // properties
    const EscherProperties &rOpts = rProps.GetOpts();
    for ( EscherProperties::const_iterator it = rOpts.begin(); it != rOpts.end(); ++it )
    {
        sal_uInt16 nId = ( it->nPropId & 0x0FFF );

        switch ( nId )
        {
            case ESCHER_Prop_WrapText:
                {
                    int nWrapType = 0;
                    switch ( it->nPropValue )
                    {
                        case ESCHER_WrapSquare:    nWrapType = 2; break;
                        case ESCHER_WrapByPoints:  nWrapType = 4; break;
                        case ESCHER_WrapNone:      nWrapType = 3; break;
                        case ESCHER_WrapTopBottom: nWrapType = 1; break;
                        case ESCHER_WrapThrough:   nWrapType = 5; break;
                    }
                    if ( nWrapType )
                        m_pShapeStyle->append(OOO_STRING_SVTOOLS_RTF_SHPWR).append((sal_Int32)nWrapType);
                }
                break;
            case ESCHER_Prop_fillColor:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("fillColor"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_fillBackColor:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("fillBackColor"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_AnchorText:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("anchorText"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_fNoFillHitTest:
                if (it->nPropValue)
                    m_aShapeProps.insert(std::pair<OString,OString>(OString("fNoFillHitTest"), OString::valueOf(sal_Int32(1))));
                break;
            case ESCHER_Prop_fNoLineDrawDash:
                // for some reason the value is set to 0x90000 if lines are switched off
                if( it->nPropValue == 0x90000 )
                    m_aShapeProps.insert(std::pair<OString,OString>(OString("fLine"), OString::valueOf(sal_Int32(0))));
                break;
            case ESCHER_Prop_lineColor:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("lineColor"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_lineBackColor:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("lineBackColor"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_lineJoinStyle:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("lineJoinStyle"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_fshadowObscured:
                if (it->nPropValue)
                    m_aShapeProps.insert(std::pair<OString,OString>(OString("fshadowObscured"), OString::valueOf(sal_Int32(1))));
                break;
            case ESCHER_Prop_geoLeft:
            case ESCHER_Prop_geoTop:
                {
                    sal_uInt32 nLeft = 0, nTop = 0;

                    if ( nId == ESCHER_Prop_geoLeft )
                    {
                        nLeft = it->nPropValue;
                        rProps.GetOpt( ESCHER_Prop_geoTop, nTop );
                    }
                    else
                    {
                        nTop = it->nPropValue;
                        rProps.GetOpt( ESCHER_Prop_geoLeft, nLeft );
                    }

                    m_aShapeProps.insert(std::pair<OString,OString>(OString("geoLeft"),
                                OString::valueOf(sal_Int32(sal_Int32( nLeft )))));
                    m_aShapeProps.insert(std::pair<OString,OString>(OString("geoTop"),
                                OString::valueOf(sal_Int32(sal_Int32( nTop )))));
                }
                break;

            case ESCHER_Prop_geoRight:
            case ESCHER_Prop_geoBottom:
                {
                    sal_uInt32 nLeft = 0, nRight = 0, nTop = 0, nBottom = 0;
                    rProps.GetOpt( ESCHER_Prop_geoLeft, nLeft );
                    rProps.GetOpt( ESCHER_Prop_geoTop, nTop );

                    if ( nId == ESCHER_Prop_geoRight )
                    {
                        nRight = it->nPropValue;
                        rProps.GetOpt( ESCHER_Prop_geoBottom, nBottom );
                    }
                    else
                    {
                        nBottom = it->nPropValue;
                        rProps.GetOpt( ESCHER_Prop_geoRight, nRight );
                    }

                    m_aShapeProps.insert(std::pair<OString,OString>(OString("geoRight"),
                                OString::valueOf(sal_Int32(sal_Int32( nRight ) - sal_Int32( nLeft )))));
                    m_aShapeProps.insert(std::pair<OString,OString>(OString("geoBottom"),
                                OString::valueOf(sal_Int32(sal_Int32( nBottom ) - sal_Int32( nTop )))));
                }
                break;
            case ESCHER_Prop_pVertices:
            case ESCHER_Prop_pSegmentInfo:
                {
                    EscherPropSortStruct aVertices;
                    EscherPropSortStruct aSegments;

                    if ( rProps.GetOpt( ESCHER_Prop_pVertices, aVertices ) &&
                         rProps.GetOpt( ESCHER_Prop_pSegmentInfo, aSegments ) )
                    {
                        const sal_uInt8 *pVerticesIt = aVertices.pBuf + 6;
                        const sal_uInt8 *pSegmentIt = aSegments.pBuf;

                        OStringBuffer aSegmentInfo( 512 );
                        OStringBuffer aVerticies( 512 );

                        sal_uInt16 nPointSize = aVertices.pBuf[4] + ( aVertices.pBuf[5] << 8 );

                        // number of segments
                        sal_uInt16 nSegments = impl_GetUInt16( pSegmentIt );
                        sal_Int32 nVertices = 0;
                        aSegmentInfo.append("2;").append((sal_Int32)nSegments);
                        pSegmentIt += 4;

                        for ( ; nSegments; --nSegments )
                        {
                            sal_uInt16 nSeg = impl_GetUInt16( pSegmentIt );
                            aSegmentInfo.append(';').append((sal_Int32)nSeg);
                            switch ( nSeg )
                            {
                                case 0x0001: // lineto
                                case 0x4000: // moveto
                                    {
                                        sal_Int32 nX = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        sal_Int32 nY = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        aVerticies.append( ";(" ).append( nX ).append( "," ).append( nY ).append( ")" );
                                        nVertices ++;
                                    }
                                    break;
                                case 0x2001: // curveto
                                    {
                                        for (int i = 0; i < 3; i++)
                                        {
                                            sal_Int32 nX = impl_GetPointComponent( pVerticesIt, nPointSize );
                                            sal_Int32 nY = impl_GetPointComponent( pVerticesIt, nPointSize );
                                            aVerticies.append( ";(" ).append( nX ).append( "," ).append( nY ).append( ")" );
                                            nVertices ++;
                                        }
                                    }
                                    break;
                                case 0xb300:
                                case 0xac00:
                                case 0xaa00: // nofill
                                case 0xab00: // nostroke
                                case 0x6001: // close
                                case 0x8000: // end
                                    break;
                                default:
                                    OSL_TRACE("%s: unhandled segment '%x' in the path", OSL_THIS_FUNC, nSeg);
                                    break;
                            }
                        }

                        if (aVerticies.getLength() )
                        {
                            // We know the number of vertices at the end only, so we have to prepend them here.
                            OStringBuffer aBuf;
                            aBuf.append("8;").append((sal_Int32)nVertices);
                            aBuf.append(aVerticies.makeStringAndClear());
                            m_aShapeProps.insert(std::pair<OString,OString>(OString("pVerticies"), aBuf.makeStringAndClear()));
                        }
                        if ( aSegmentInfo.getLength() )
                            m_aShapeProps.insert(std::pair<OString,OString>(OString("pSegmentInfo"), aSegmentInfo.makeStringAndClear()));
                    }
                    else
                        OSL_TRACE("%s: unhandled shape path, missing either pVertices or pSegmentInfo", OSL_THIS_FUNC);
                }
                break;
            case ESCHER_Prop_shapePath:
                // noop, we use pSegmentInfo instead
                break;
            case ESCHER_Prop_fFillOK:
                if (!it->nPropValue)
                    m_aShapeProps.insert(std::pair<OString,OString>(OString("fFillOK"), OString::valueOf(sal_Int32(0))));
                break;
            case ESCHER_Prop_dxTextLeft:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("dxTextLeft"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_dyTextTop:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("dyTextTop"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_dxTextRight:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("dxTextRight"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_dyTextBottom:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("dyTextBottom"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_FitTextToShape:
                // Size text to fit shape size: not supported by RTF
                break;
            case ESCHER_Prop_adjustValue:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("adjustValue"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            case ESCHER_Prop_txflTextFlow:
                m_aShapeProps.insert(std::pair<OString,OString>(OString("txflTextFlow"), OString::valueOf(sal_Int32(it->nPropValue))));
                break;
            default:
                OSL_TRACE("%s: unhandled property: %d (value: %d)", OSL_THIS_FUNC, nId, it->nPropValue);
                break;
        }
    }
}

void RtfSdrExport::AddLineDimensions( const Rectangle& rRectangle )
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    // We get the position relative to (the current?) character
    m_aShapeProps.insert(std::pair<OString,OString>(OString("posrelh"), OString::valueOf(sal_Int32(3))));

    switch ( m_nShapeFlags & 0xC0 )
    {
        case 0x40:
            m_aShapeProps.insert(std::pair<OString,OString>(OString("fFlipV"), OString::valueOf(sal_Int32(1))));
            break;
        case 0x80:
            m_aShapeProps.insert(std::pair<OString,OString>(OString("fFlipH"), OString::valueOf(sal_Int32(1))));
            break;
        case 0xC0:
            m_aShapeProps.insert(std::pair<OString,OString>(OString("fFlipV"), OString::valueOf(sal_Int32(1))));
            m_aShapeProps.insert(std::pair<OString,OString>(OString("fFlipH"), OString::valueOf(sal_Int32(1))));
            break;
    }

    // the actual dimensions
    m_pShapeStyle->append(OOO_STRING_SVTOOLS_RTF_SHPLEFT).append(rRectangle.Left());
    m_pShapeStyle->append(OOO_STRING_SVTOOLS_RTF_SHPTOP).append(rRectangle.Top());
    m_pShapeStyle->append(OOO_STRING_SVTOOLS_RTF_SHPRIGHT).append(rRectangle.Right());
    m_pShapeStyle->append(OOO_STRING_SVTOOLS_RTF_SHPBOTTOM).append(rRectangle.Bottom());
}

void RtfSdrExport::AddRectangleDimensions( rtl::OStringBuffer& rBuffer, const Rectangle& rRectangle )
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    // We get the position relative to (the current?) character
    m_aShapeProps.insert(std::pair<OString,OString>(OString("posrelh"), OString::valueOf(sal_Int32(3))));

    rBuffer.append(OOO_STRING_SVTOOLS_RTF_SHPLEFT).append(rRectangle.Left());
    rBuffer.append(OOO_STRING_SVTOOLS_RTF_SHPTOP).append(rRectangle.Top());
    rBuffer.append(OOO_STRING_SVTOOLS_RTF_SHPRIGHT).append(rRectangle.Right());
    rBuffer.append(OOO_STRING_SVTOOLS_RTF_SHPBOTTOM).append(rRectangle.Bottom());
}

void RtfSdrExport::AddShapeAttribute( sal_Int32 /*nAttribute*/, const rtl::OString& /*rValue*/ )
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    /* noop */
}

extern const char* pShapeTypes[];

void lcl_AppendSP( ::rtl::OStringBuffer& rRunText, const char cName[], const ::rtl::OString& rValue)
{
    rRunText.append('{').append(OOO_STRING_SVTOOLS_RTF_SP)
        .append('{').append(OOO_STRING_SVTOOLS_RTF_SN " ").append(cName).append('}')
        .append('{').append(OOO_STRING_SVTOOLS_RTF_SV " ").append(rValue).append('}')
        .append('}');
}
sal_Int32 RtfSdrExport::StartShape()
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    if ( m_nShapeType == ESCHER_ShpInst_Nil )
        return -1;

    m_aShapeProps.insert(std::pair<OString,OString>(OString("shapeType"), OString::valueOf(sal_Int32(m_nShapeType))));

    m_rAttrOutput.RunText().append('{').append(OOO_STRING_SVTOOLS_RTF_SHP);
    m_rAttrOutput.RunText().append('{').append(OOO_STRING_SVTOOLS_RTF_IGNORE).append(OOO_STRING_SVTOOLS_RTF_SHPINST);

    m_rAttrOutput.RunText().append(m_pShapeStyle->makeStringAndClear());
    // Ignore \shpbxpage, \shpbxmargin, and \shpbxcolumn, in favor of the posrelh property.
    m_rAttrOutput.RunText().append(OOO_STRING_SVTOOLS_RTF_SHPBXIGNORE);
    // Ignore \shpbypage, \shpbymargin, and \shpbycolumn, in favor of the posrelh property.
    m_rAttrOutput.RunText().append(OOO_STRING_SVTOOLS_RTF_SHPBYIGNORE);

    for(std::map<OString,OString>::reverse_iterator i = m_aShapeProps.rbegin(); i != m_aShapeProps.rend(); i++)
        lcl_AppendSP(m_rAttrOutput.RunText(), (*i).first, (*i).second );

    lcl_AppendSP(m_rAttrOutput.RunText(), "wzDescription", RtfExport::OutString( m_pSdrObject->GetDescription(), m_rExport.eCurrentEncoding));
    lcl_AppendSP(m_rAttrOutput.RunText(), "wzName", RtfExport::OutString( m_pSdrObject->GetTitle(), m_rExport.eCurrentEncoding));

    // now check if we have some text
    const SdrTextObj* pTxtObj = PTR_CAST(SdrTextObj, m_pSdrObject);
    if (pTxtObj)
    {
        const OutlinerParaObject* pParaObj = 0;
        bool bOwnParaObj = false;

        /*
        #i13885#
        When the object is actively being edited, that text is not set into
        the objects normal text object, but lives in a seperate object.
        */
        if (pTxtObj->IsTextEditActive())
        {
            pParaObj = pTxtObj->GetEditOutlinerParaObject();
            bOwnParaObj = true;
        }
        else
        {
            pParaObj = pTxtObj->GetOutlinerParaObject();
        }

        if( pParaObj )
        {
            // this is reached only in case some text is attached to the shape
            WriteOutliner(*pParaObj);
            if( bOwnParaObj )
                delete pParaObj;
        }
    }

    return m_nShapeType;
}

void RtfSdrExport::WriteOutliner(const OutlinerParaObject& rParaObj)
{
    OSL_TRACE("%s start", OSL_THIS_FUNC);

    const EditTextObject& rEditObj = rParaObj.GetTextObject();
    MSWord_SdrAttrIter aAttrIter( m_rExport, rEditObj, TXT_HFTXTBOX );

    sal_uInt16 nPara = rEditObj.GetParagraphCount();

    m_rAttrOutput.RunText().append('{').append(OOO_STRING_SVTOOLS_RTF_SHPTXT).append(' ');
    for (sal_uInt16 n = 0; n < nPara; ++n)
    {
        if( n )
            aAttrIter.NextPara( n );

        rtl_TextEncoding eChrSet = aAttrIter.GetNodeCharSet();

        String aStr( rEditObj.GetText( n ));
        xub_StrLen nAktPos = 0;
        xub_StrLen nEnd = aStr.Len();

        aAttrIter.OutParaAttr(false);
        m_rAttrOutput.RunText().append(m_rAttrOutput.Styles().makeStringAndClear());

        do {
            xub_StrLen nNextAttr = aAttrIter.WhereNext();
            rtl_TextEncoding eNextChrSet = aAttrIter.GetNextCharSet();

            if( nNextAttr > nEnd )
                nNextAttr = nEnd;

            aAttrIter.OutAttr( nAktPos );
            m_rAttrOutput.RunText().append('{').append(m_rAttrOutput.Styles().makeStringAndClear()).append(m_rExport.sNewLine);
            bool bTxtAtr = aAttrIter.IsTxtAttr( nAktPos );
            if( !bTxtAtr )
            {
                String aOut( aStr.Copy( nAktPos, nNextAttr - nAktPos ) );
                m_rAttrOutput.RunText().append( m_rExport.OutString( aOut, eChrSet ) );
            }

            m_rAttrOutput.RunText().append('}');

            nAktPos = nNextAttr;
            eChrSet = eNextChrSet;
            aAttrIter.NextPos();
        }
        while( nAktPos < nEnd );
    }
    m_rAttrOutput.RunText().append(OOO_STRING_SVTOOLS_RTF_PAR).append('}');

    OSL_TRACE("%s end", OSL_THIS_FUNC);
}

void RtfSdrExport::EndShape( sal_Int32 nShapeElement )
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    if ( nShapeElement >= 0 )
    {
        // end of the shape
        m_rAttrOutput.RunText().append('}').append('}');
    }
}

sal_uInt32 RtfSdrExport::AddSdrObject( const SdrObject& rObj )
{
    m_pSdrObject = &rObj;
    return EscherEx::AddSdrObject(rObj);
}

/* vi:set shiftwidth=4 expandtab: */
