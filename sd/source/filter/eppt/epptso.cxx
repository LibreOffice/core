/*************************************************************************
 *
 *  $RCSfile: epptso.cxx,v $
 *
 *  $Revision: 1.37 $
 *
 *  last change: $Author: sj $ $Date: 2001-04-04 15:57:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EPPT_HXX_
#include <eppt.hxx>
#endif
#ifndef _EPPT_DEF_HXX
#include "epptdef.hxx"
#endif
#ifndef _PptEscherEx_HXX
#include "escherex.hxx"
#endif
#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
#ifndef _SV_GFXLINK_HXX
#include <vcl/gfxlink.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
//#ifndef _SVX_XIT_HXX
//#include <svx/xit.hxx>
//#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_
#include <com/sun/star/style/VerticalAlignment.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONPAGE_HPP_
#include <com/sun/star/presentation/XPresentationPage.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONT_HPP_
#include <com/sun/star/awt/XFont.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHADJUST_HPP_
#include <com/sun/star/style/ParagraphAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACINGMODE_HPP_
#include <com/sun/star/style/LineSpacingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_PP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FLAGSEQUENCE_HPP_
#include <com/sun/star/drawing/FlagSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYGONFLAGS_HPP_
#include <com/sun/star/drawing/PolygonFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_PROPTYPEHLP_HXX_
#include <cppuhelper/proptypehlp.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX_
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX_
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/unohlp.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif

#include <svtools/fltcall.hxx>

//#include <svx/xbtmpit.hxx>


#include <vos/xception.hxx>
#ifndef _VOS_NO_NAMESPACE
using namespace vos;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Font Families */
#define FF_DONTCARE             0x00
#define FF_ROMAN                0x10
#define FF_SWISS                0x20
#define FF_MODERN               0x30
#define FF_SCRIPT               0x40
#define FF_DECORATIVE           0x50

////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_PITCH           0x00
#define FIXED_PITCH             0x01
#define VARIABLE_PITCH          0x02

// ---------------------------------------------------------------------------------------------

sal_Int16 EncodeAnyTosal_Int16( ::com::sun::star::uno::Any& rAny )
{
        sal_Int16 nVal = 0;
#ifdef __BIGENDIAN
        switch( rAny.getValueType().getTypeClass() )
        {
            case ::com::sun::star::uno::TypeClass_SHORT :
            case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT : nVal = *((short*)rAny.getValue() ); break;
            case ::com::sun::star::uno::TypeClass_LONG :
            case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG : nVal = (sal_Int16)(*((long*)rAny.getValue() ) ); break;
            case ::com::sun::star::uno::TypeClass_ENUM :
            {
                 nVal = (sal_Int16)(*( ::com::sun::star::drawing::TextAdjust*)rAny.getValue() ); break;
            }
            break;
        }
#else
        nVal = *((sal_Int16*)rAny.getValue());
#endif
        return nVal;
}

PPTExBulletProvider::PPTExBulletProvider()
{
    pGraphicProv = new EscherGraphicProvider( _E_GRAPH_PROV_USE_INSTANCES  | _E_GRAPH_PROV_DO_NOT_ROTATE_METAFILES );
}

PPTExBulletProvider::~PPTExBulletProvider()
{
    delete pGraphicProv;
}

sal_uInt16 PPTExBulletProvider::GetId( const ByteString& rUniqueId, Size& rGraphicSize )
{
    sal_uInt16 nRetValue = 0xffff;
    sal_uInt32 nId = 0;

    if ( rUniqueId.Len() )
    {
        GraphicObject   aGraphicObject( rUniqueId );
        Graphic         aGraph( aGraphicObject.GetGraphic() );
        Size            aPrefSize( aGraph.GetPrefSize() );
        double          fQ1 = ( (double)aPrefSize.Width() / (double)aPrefSize.Height() );
        double          fQ2 = ( (double)rGraphicSize.Width() / (double)rGraphicSize.Height() );
        double          fXScale = 1;
        double          fYScale = 1;

        if ( fQ1 > fQ2 )
            fYScale = fQ1 / fQ2;
        else if ( fQ1 < fQ2 )
            fXScale = fQ2 / fQ1;

        Rectangle       aRect;
        Graphic         aGraphic( aGraphicObject.GetGraphic() );
        BitmapEx        aBmpEx( aGraphic.GetBitmapEx() );
        if ( ( fXScale != 1.0 ) || ( fYScale != 1.0 ) )
        {
            aBmpEx.Scale( fXScale, fYScale );
            Size aNewSize( (sal_Int32)((double)rGraphicSize.Width() / fXScale + 0.5 ),
                            (sal_Int32)((double)rGraphicSize.Height() / fYScale + 0.5 ) );
            rGraphicSize = aNewSize;
        }
        Graphic         aBmpGraphic( aBmpEx );
        GraphicObject   aMappedGraphicObject( aBmpGraphic );

        nId = pGraphicProv->GetBlibID( aBuExPictureStream, aMappedGraphicObject.GetUniqueID(), aRect, NULL );

        if ( nId && ( nId < 0x10000 ) )
            nRetValue = (sal_uInt16)nId - 1;
    }
    return nRetValue;
}

// ---------------------------------------------------------------------------------------------

GroupTable::GroupTable() :
    mpGroupEntry        ( NULL ),
    mnMaxGroupEntry     ( 0 ),
    mnCurrentGroupEntry ( 0 ),
    mnGroupsClosed      ( 0 )
{
    ImplResizeGroupTable( 32 );
}

// ---------------------------------------------------------------------------------------------

GroupTable::~GroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; delete mpGroupEntry[ i++ ] );
    delete mpGroupEntry;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ImplResizeGroupTable( sal_uInt32 nEntrys )
{
    if ( nEntrys > mnMaxGroupEntry )
    {
        mnMaxGroupEntry         = nEntrys;
        GroupEntry** pTemp = new GroupEntry*[ nEntrys ];
        for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++ )
            pTemp[ i ] = mpGroupEntry[ i ];
        if ( mpGroupEntry )
            delete mpGroupEntry;
        mpGroupEntry = pTemp;
    }
}

// ---------------------------------------------------------------------------------------------

void GroupTable::EnterGroup( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rXIndexAccessRef )
{
    if ( rXIndexAccessRef.is() )
    {
        if ( mnMaxGroupEntry == mnCurrentGroupEntry )
            ImplResizeGroupTable( mnMaxGroupEntry + 8 );
        mpGroupEntry[ mnCurrentGroupEntry++ ] = new GroupEntry( rXIndexAccessRef );
    }
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 GroupTable::GetGroupsClosed()
{
    sal_uInt32 nRet = mnGroupsClosed;
    mnGroupsClosed = 0;
    return nRet;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ClearGroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++, delete mpGroupEntry );
    mnCurrentGroupEntry = 0;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ResetGroupTable( sal_uInt32 nCount )
{
    ClearGroupTable();
    mpGroupEntry[ mnCurrentGroupEntry++ ] = new GroupEntry( nCount );
}

// ---------------------------------------------------------------------------------------------

sal_Bool GroupTable::GetNextGroupEntry()
{
    while ( mnCurrentGroupEntry )
    {
        mnIndex = mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCurrentPos++;

        if ( mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCount > mnIndex )
            return TRUE;

        delete ( mpGroupEntry[ --mnCurrentGroupEntry ] );

        if ( mnCurrentGroupEntry )
            mnGroupsClosed++;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::SkipCurrentGroup()
{
    if ( mnCurrentGroupEntry )
        delete ( mpGroupEntry[ --mnCurrentGroupEntry ] );
}

// ---------------------------------------------------------------------------------------------

SoundEntry::SoundEntry( const String& rString ) :
    aSoundURL( rString ),
    nFileSize( 0 )
{
    try
    {
        ::ucb::Content aCnt( aSoundURL,
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
        sal_Int64 nVal;
        ::cppu::convertPropertyValue( nVal, aCnt.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ) ) );
        nFileSize = (sal_uInt32)nVal;
    }
    catch( ::com::sun::star::uno::Exception& )
    {

    }
};

String SoundEntry::ImplGetName() const
{
    INetURLObject aTmp( aSoundURL );
    return aTmp.GetName();
}

String SoundEntry::ImplGetExtension() const
{
    INetURLObject aTmp( aSoundURL );
    String aExtension( aTmp.GetExtension() );
    if ( aExtension.Len() )
        aExtension.Insert( (sal_Unicode)'.', 0 );
    return aExtension;
}

sal_Bool SoundEntry::IsSameURL( const String& rURL ) const
{
    return ( rURL == aSoundURL );
}

sal_uInt32 SoundEntry::GetSize( sal_uInt32 nId ) const
{
    String aName( ImplGetName() );
    String aExtension( ImplGetExtension() );

    sal_uInt32 nSize = 8;                           // SoundContainer Header
    if ( aName.Len() )                              // String Atom          ( instance 0 - name of sound )
        nSize += aName.Len() * 2 + 8;
    if ( aExtension.Len() )                         // String Atom          ( instance 1 - extension of sound )
        nSize += aExtension.Len() * 2 + 8;

    String aId( String::CreateFromInt32( nId ) );   // String Atom          ( instance 2 - reference id )
    nSize += 2 * aId.Len() + 8;

    nSize += nFileSize + 8;                         // SoundData Atom

    return nSize;
}

void SoundEntry::Write( SvStream& rSt, sal_uInt32 nId )
{
    try
    {
        ::ucb::Content aCnt( aSoundURL,
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

        // create SoundContainer
        rSt << (sal_uInt32)( ( EPP_Sound << 16 ) | 0xf ) << (sal_uInt32)( GetSize( nId ) - 8 );

        String aSoundName( ImplGetName() );
        sal_uInt16 i, nSoundNameLen = aSoundName.Len();
        if ( nSoundNameLen )
        {
            // name of sound ( instance 0 )
            rSt << (sal_uInt32)( EPP_CString << 16 ) << (sal_uInt32)( nSoundNameLen * 2 );
            for ( i = 0; i < nSoundNameLen; i++ )
                rSt << aSoundName.GetChar( i );
        }
        String aExtension( ImplGetExtension() );
        sal_uInt32 nExtensionLen = aExtension.Len();
        if ( nExtensionLen )
        {
            // extension of sound ( instance 1 )
            rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 16 ) << (sal_uInt32)( nExtensionLen * 2 );
            for ( i = 0; i < nExtensionLen; i++ )
                rSt << aExtension.GetChar( i );
        }
        // id of sound ( instance 2 )
        String aId( String::CreateFromInt32( nId ) );
        sal_uInt32 nIdLen = aId.Len();
        rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 32 ) << (sal_uInt32)( nIdLen * 2 );
        for ( i = 0; i < nIdLen; i++ )
            rSt << aId.GetChar( i );

        rSt << (sal_uInt32)( EPP_SoundData << 16 ) << (sal_uInt32)( nFileSize );
        sal_uInt32 nBytesLeft = nFileSize;
        SvStream* pSourceFile = ::utl::UcbStreamHelper::CreateStream( aSoundURL, STREAM_READ );
        if ( pSourceFile )
        {
            sal_uInt8* pBuf = new sal_uInt8[ 0x10000 ];   // 64 kB  Buffer
            while ( nBytesLeft )
            {
                sal_uInt32 nToDo = ( nBytesLeft > 0x10000 ) ? 0x10000 : nBytesLeft;
                pSourceFile->Read( pBuf, nToDo );
                rSt.Write( pBuf, nToDo );
                nBytesLeft -= nToDo;
            }
            delete pSourceFile;
            delete pBuf;
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {

    }
}

SoundCollection::~SoundCollection()
{
    for( void* pPtr = List::First(); pPtr; pPtr = List::Next() )
        delete (SoundEntry*)pPtr;
}

sal_uInt32 SoundCollection::GetId( const String& rString )
{
    sal_uInt32 nSoundId = 0;
    if( rString.Len() )
    {
        const sal_uInt32 nCount = Count();

        for( ; nSoundId < nCount; nSoundId++ )
            if( ImplGetByIndex( nSoundId )->IsSameURL( rString ) )
                break;
        if ( nSoundId++ == nCount )
        {
            SoundEntry* pEntry = new SoundEntry( rString );
            if ( pEntry->GetFileSize() )
                List::Insert( pEntry, LIST_APPEND );
            else
            {
                nSoundId = 0;   // only insert sounds that are accessible
                delete pEntry;
            }
        }
    }
    return nSoundId;
}

const SoundEntry* SoundCollection::ImplGetByIndex( sal_uInt32 nIndex ) const
{
    return (SoundEntry*)List::GetObject( nIndex );
}

sal_uInt32 SoundCollection::GetSize() const
{
    sal_uInt32 nSize = 0;
    sal_uInt32 i, nCount = Count();
    if ( nCount )
    {
        nSize += 8 + 12;    // size of SoundCollectionContainerHeader + SoundCollAtom
        for ( i = 0; i < nCount; i++ )
            nSize += ImplGetByIndex( i )->GetSize( i + 1 );
    }
    return nSize;
}

void SoundCollection::Write( SvStream& rSt )
{
    sal_uInt32 i, nCount = Count();
    if ( nCount )
    {
        // create SoundCollection Container
        rSt << (sal_uInt16)0xf << (sal_uInt16)EPP_SoundCollection << (sal_uInt32)( GetSize() - 8 );

        // create SoundCollAtom ( reference to the next free SoundId );
        rSt << (sal_uInt32)( EPP_SoundCollAtom << 16 ) << (sal_uInt32)4 << nCount;

        for ( i = 0; i < nCount; i++ )
            ((SoundEntry*)List::GetObject( i ))->Write( rSt, i + 1 );
    }
}

// ---------------------------------------------------------------------------------------------

FontCollection::~FontCollection()
{
    for( void* pStr = List::First(); pStr; pStr = List::Next() )
        delete (FontCollectionEntry*)pStr;
}

FontCollection::FontCollection()
{

}

sal_uInt32 FontCollection::GetId( const FontCollectionEntry& rEntry )
{
    if( rEntry.Name.Len() )
    {
        const sal_uInt32 nCount = GetCount();

        for( sal_uInt32 i = 0; i < nCount; i++ )
            if( GetById( i )->Name == rEntry.Name )
                return i;

        List::Insert( new FontCollectionEntry( rEntry ), LIST_APPEND );
        return nCount;
    }
    return 0;
}

const FontCollectionEntry* FontCollection::GetById( sal_uInt32 nId )
{
    return (FontCollectionEntry*)List::GetObject( nId );
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplVBAInfoContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 28;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0x1f | ( EPP_VBAInfo << 16 ) )
               << (sal_uInt32)( nSize - 8 )
               << (sal_uInt32)( 2 | ( EPP_VBAInfoAtom << 16 ) )
               << (sal_uInt32)12;
        mpPptEscherEx->InsertPersistOffset( EPP_Persist_VBAInfoAtom, pStrm->Tell() );
        *pStrm << (sal_uInt32)0
               << (sal_uInt32)0
               << (sal_uInt32)1;
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplSlideViewInfoContainer( sal_uInt32 nInstance, SvStream* pStrm )
{
    sal_uInt32 nSize = 111;
    if ( pStrm )
    {
        sal_uInt8 bShowGuides = 0;
        sal_uInt8 bSnapToGrid = 1;
        sal_uInt8 bSnapToShape = 0;

        sal_Int32 nScaling = 85;
        sal_Int32 nMasterCoordinate = 0xdda;
        sal_Int32 nXOrigin = -780;
        sal_Int32 nYOrigin = -84;

        sal_Int32 nPosition1 = 0x870;
        sal_Int32 nPosition2 = 0xb40;

        if ( nInstance )
        {
            bShowGuides = 1;
            nScaling = 0x3b;
            nMasterCoordinate = 0xf0c;
            nXOrigin = -1752;
            nYOrigin = -72;
            nPosition1 = 0xb40;
            nPosition2 = 0x870;
        }
        *pStrm << (sal_uInt32)( 0xf | ( EPP_SlideViewInfo << 16 ) | ( nInstance << 4 ) )
               << (sal_uInt32)( nSize - 8 )
               << (sal_uInt32)( EPP_SlideViewInfoAtom << 16 ) << (sal_uInt32)3
               << bShowGuides << bSnapToGrid << bSnapToShape
               << (sal_uInt32)( EPP_ViewInfoAtom << 16 ) << (sal_uInt32)52
               << nScaling << (sal_Int32)100 << nScaling << (sal_Int32)100  // scaling atom - Keeps the current scale
               << nScaling << (sal_Int32)100 << nScaling << (sal_Int32)100  // scaling atom - Keeps the previous scale
               << (sal_Int32)0x17ac << nMasterCoordinate// Origin - Keeps the origin in master coordinates
               << nXOrigin << nYOrigin              // Origin
               << (sal_uInt8)1                          // Bool1 varScale - Set if zoom to fit is set
               << (sal_uInt8)0                          // bool1 draftMode - Not used
               << (sal_uInt16)0                         // padword
               << (sal_uInt32)( ( 7 << 4 ) | ( EPP_GuideAtom << 16 ) ) << (sal_uInt32)8
               << (sal_uInt32)0     // Type of the guide. If the guide is horizontal this value is zero. If it's vertical, it's one.
               << nPosition1    // Position of the guide in master coordinates. X coordinate if it's vertical, and Y coordinate if it's horizontal.
               << (sal_uInt32)( ( 7 << 4 ) | ( EPP_GuideAtom << 16 ) ) << (sal_uInt32)8
               << (sal_Int32)1      // Type of the guide. If the guide is horizontal this value is zero. If it's vertical, it's one.
               << nPosition2;   // Position of the guide in master coordinates. X coordinate if it's vertical, and Y coordinate if it's horizontal.
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplOutlineViewInfoContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 68;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0xf | ( EPP_OutlineViewInfo << 16 ) ) << (sal_uInt32)( nSize - 8 )
               << (sal_uInt32)( EPP_ViewInfoAtom << 16 ) << (sal_uInt32)52
               << (sal_Int32)170 << (sal_Int32)200 << (sal_Int32)170 << (sal_Int32)200  // scaling atom - Keeps the current scale
               << (sal_Int32)170 << (sal_Int32)200 << (sal_Int32)170 << (sal_Int32)200  // scaling atom - Keeps the previous scale
               << (sal_Int32)0x17ac << 0xdda    // Origin - Keeps the origin in master coordinates
               << (sal_Int32)-780 << (sal_Int32)-84 // Origin
               << (sal_uInt8)1                  // bool1 varScale - Set if zoom to fit is set
               << (sal_uInt8)0                  // bool1 draftMode - Not used
               << (sal_uInt16)0;                // padword
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplProgBinaryTag( SvStream* pStrm )
{
    sal_uInt32 nPictureStreamSize, nOutlineStreamSize, nSize = 8;

    nPictureStreamSize = aBuExPictureStream.Tell();
    if ( nPictureStreamSize )
        nSize += nPictureStreamSize + 8;

    nOutlineStreamSize = aBuExOutlineStream.Tell();
    if ( nOutlineStreamSize )
        nSize += nOutlineStreamSize + 8;

    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( EPP_BinaryTagData << 16 ) << (sal_uInt32)( nSize - 8 );
        if ( nPictureStreamSize )
        {
            *pStrm << (sal_uInt32)( 0xf | ( EPP_PST_ExtendedBuGraContainer << 16 ) ) << nPictureStreamSize;
            pStrm->Write( aBuExPictureStream.GetData(), nPictureStreamSize );
        }
        if ( nOutlineStreamSize )
        {
            *pStrm << (sal_uInt32)( 0xf | ( EPP_PST_ExtendedPresRuleContainer << 16 ) ) << nOutlineStreamSize;
            pStrm->Write( aBuExOutlineStream.GetData(), nOutlineStreamSize );
        }
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplProgBinaryTagContainer( SvStream* pStrm, SvMemoryStream* pBinTagStrm )
{
    sal_uInt32 nSize = 8 + 8 + 14;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0xf | ( EPP_ProgBinaryTag << 16 ) ) << (sal_uInt32)0
               << (sal_uInt32)( EPP_CString << 16 ) << (sal_uInt32)14
               << (sal_uInt32)0x5f005f << (sal_uInt32)0x50005f
               << (sal_uInt32)0x540050 << (sal_uInt16)0x39;
    }
    if ( pBinTagStrm )
    {
        sal_uInt32 nLen = pBinTagStrm->Tell();
        nSize += nLen + 8;
        *pStrm << (sal_uInt32)( EPP_BinaryTagData << 16 ) << nLen;
        pStrm->Write( pBinTagStrm->GetData(), nLen );
    }
    else
        nSize += ImplProgBinaryTag( pStrm );

    if ( pStrm )
    {
        pStrm->SeekRel( - ( (sal_Int32)nSize - 4 ) );
        *pStrm << (sal_uInt32)( nSize - 8 );
        pStrm->SeekRel( nSize - 8 );
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplProgTagContainer( SvStream* pStrm, SvMemoryStream* pBinTagStrm )
{
    sal_uInt32 nSize = 0;
    if ( aBuExPictureStream.Tell() || aBuExOutlineStream.Tell() || pBinTagStrm )
    {
        nSize = 8;
        if ( pStrm )
        {
            *pStrm << (sal_uInt32)( 0xf | ( EPP_ProgTags << 16 ) ) << (sal_uInt32)0;
        }
        nSize += ImplProgBinaryTagContainer( pStrm, pBinTagStrm );
        if ( pStrm )
        {
            pStrm->SeekRel( - ( (sal_Int32)nSize - 4 ) );
            *pStrm << (sal_uInt32)( nSize - 8 );
            pStrm->SeekRel( nSize - 8 );
        }
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplDocumentListContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 8;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( ( EPP_List << 16 ) | 0xf ) << (sal_uInt32)0;
    }

    nSize += ImplVBAInfoContainer( pStrm );
    nSize += ImplSlideViewInfoContainer( 0, pStrm );
    nSize += ImplOutlineViewInfoContainer( pStrm );
    nSize += ImplSlideViewInfoContainer( 1, pStrm );
    nSize += ImplProgTagContainer( pStrm );

    if ( pStrm )
    {
        pStrm->SeekRel( - ( (sal_Int32)nSize - 4 ) );
        *pStrm << (sal_uInt32)( nSize - 8 );
        pStrm->SeekRel( nSize - 8 );
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplMasterSlideListContainer( SvStream* pStrm )
{
    sal_uInt32 i, nSize = 28 * mnMasterPages + 8;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0x1f | ( EPP_SlideListWithText << 16 ) ) << (sal_uInt32)( nSize - 8 );

        for ( i = 0; i < mnMasterPages; i++ )
        {
            *pStrm << (sal_uInt32)( EPP_SlidePersistAtom << 16 ) << (sal_uInt32)20;
            mpPptEscherEx->InsertPersistOffset( EPP_MAINMASTER_PERSIST_KEY | i, pStrm->Tell() );
            *pStrm << (sal_uInt32)0                 // psrReference - logical reference to the slide persist object ( EPP_MAINMASTER_PERSIST_KEY )
                   << (sal_uInt32)0                 // flags - only bit 3 used, if set then slide contains shapes other than placeholders
                   << (sal_Int32)0                  // numberTexts - number of placeholder texts stored with the persist object. Allows to display outline view without loading the slide persist objects
                   << (sal_Int32)( 0x80000000 | i ) // slideId - Unique slide identifier, used for OLE link monikers for example
                   << (sal_uInt32)0;                // reserved, usualy 0
        }
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCloseDocument()
{
    sal_uInt32 nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Document );
    if ( nOfs )
    {
        mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_CurrentPos, mpStrm->Tell() );
        mpStrm->Seek( nOfs );

        mpExEmbed->Seek( STREAM_SEEK_TO_END );
        sal_uInt32 nExEmbedSize = mpExEmbed->Tell();

        // nEnviroment : Gesamtgroesse des Environment Containers
        sal_uInt32 nEnvironment = maFontCollection.GetCount() * 76  // 68 bytes pro Fontenityatom und je 8 Bytes fuer die Header
                                + 8                             // 1 FontCollection Container
                                + 20                            // SrKinsoku Container
                                + 18                            // 1 TxSiStyleAtom
                                + 118;                          // 1 TxMasterStyleAtom;

        sal_uInt32 nBytesToInsert = nEnvironment + 8;

        if ( nExEmbedSize )
            nBytesToInsert += nExEmbedSize + 8 + 12;

        nBytesToInsert += mpStyleSheet->SizeOfTxCFStyleAtom();
        nBytesToInsert += maSoundCollection.GetSize();
        nBytesToInsert += mpPptEscherEx->DrawingGroupContainerSize();
        nBytesToInsert += ImplMasterSlideListContainer( NULL );
        nBytesToInsert += ImplDocumentListContainer( NULL );

        // nBytes im Stream einfuegen, und abhaengige Container anpassen
        mpPptEscherEx->InsertAtCurrentPos( nBytesToInsert, TRUE );

        // CREATE HYPERLINK CONTAINER
        if ( nExEmbedSize )
        {
            *mpStrm << (sal_uInt16)0xf
                    << (sal_uInt16)EPP_ExObjList
                    << (sal_uInt32)( nExEmbedSize + 12 )
                    << (sal_uInt16)0
                    << (sal_uInt16)EPP_ExObjListAtom
                    << (sal_uInt32)4
                    << (sal_uInt32)mnExEmbed;
            mpPptEscherEx->InsertPersistOffset( EPP_Persist_ExObj, mpStrm->Tell() );
            mpStrm->Write( mpExEmbed->GetData(), nExEmbedSize );
        }

        // CREATE ENVIRONMENT
        *mpStrm << (sal_uInt16)0xf << (sal_uInt16)EPP_Environment << (sal_uInt32)nEnvironment;

        // Open Container ( EPP_SrKinsoku )
        *mpStrm << (sal_uInt16)0x2f << (sal_uInt16)EPP_SrKinsoku << (sal_uInt32)12;
        mpPptEscherEx->AddAtom( 4, EPP_SrKinsokuAtom, 0, 3 );
        *mpStrm << (sal_Int32)0;                        // SrKinsoku Level 0

        // Open Container ( EPP_FontCollection )
        *mpStrm << (sal_uInt16)0xf << (sal_uInt16)EPP_FontCollection << (sal_uInt32)maFontCollection.GetCount() * 76;

        for ( sal_uInt32 i = 0; i < maFontCollection.GetCount(); i++ )
        {
            mpPptEscherEx->AddAtom( 68, EPP_FontEnityAtom, 0, i );
            const FontCollectionEntry* pDesc = maFontCollection.GetById( i );
            sal_uInt32 nFontLen = pDesc->Name.Len();
            if ( nFontLen > 31 )
                nFontLen = 31;
            for ( sal_uInt16 n = 0; n < 32; n++ )
            {
                sal_Unicode nUniCode = 0;
                if ( n < nFontLen )
                    nUniCode = pDesc->Name.GetChar( n );
                *mpStrm << nUniCode;
            }
            sal_uInt8   lfCharSet = ANSI_CHARSET;
            sal_uInt8   lfClipPrecision = 0;
            sal_uInt8   lfQuality = 6;
            sal_uInt8   lfPitchAndFamily = 0;

            if ( pDesc->CharSet == RTL_TEXTENCODING_SYMBOL )
                lfCharSet = SYMBOL_CHARSET;

            switch( pDesc->Family )
            {
                case ::com::sun::star::awt::FontFamily::ROMAN :
                    lfPitchAndFamily |= FF_ROMAN;
                break;

                case ::com::sun::star::awt::FontFamily::SWISS :
                    lfPitchAndFamily |= FF_SWISS;
                break;

                case ::com::sun::star::awt::FontFamily::MODERN :
                    lfPitchAndFamily |= FF_MODERN;
                break;

                case ::com::sun::star::awt::FontFamily::SCRIPT:
                    lfPitchAndFamily |= FF_SCRIPT;
                break;

                case ::com::sun::star::awt::FontFamily::DECORATIVE:
                     lfPitchAndFamily |= FF_DECORATIVE;
                break;

                default:
                    lfPitchAndFamily |= FAMILY_DONTKNOW;
                break;
            }
            switch( pDesc->Pitch )
            {
                case ::com::sun::star::awt::FontPitch::FIXED:
                    lfPitchAndFamily |= FIXED_PITCH;
                break;

                default:
                    lfPitchAndFamily |= DEFAULT_PITCH;
                break;
            }
            *mpStrm << lfCharSet
                    << lfClipPrecision
                    << lfQuality
                    << lfPitchAndFamily;
        }
        mpStyleSheet->WriteTxCFStyleAtom( *mpStrm );        // create style that is used for new standard objects
        mpPptEscherEx->AddAtom( 10, EPP_TxSIStyleAtom );
        *mpStrm << (sal_uInt32)7                        // ?
                << (sal_Int16)2                         // ?
                << (sal_uInt8)9                          // ?
                << (sal_uInt8)8                          // ?
                << (sal_Int16)0;                        // ?
        mpPptEscherEx->AddAtom( 110, EPP_TxMasterStyleAtom, 0, 4 );
        static sal_uInt8 aTxSuStyleAtom[ 110 ] =
        {
            0x05, 0x00, 0xff, 0xfd, 0x3f, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02,
            0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xef, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x18, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05,
            0x00, 0x00, 0x20, 0x01, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x40, 0x02,
            0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x60, 0x03, 0x60, 0x03, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x80, 0x04, 0x80, 0x04, 0x00, 0x00, 0x00, 0x00
        };
        mpStrm->Write( &aTxSuStyleAtom, 110 );
        maSoundCollection.Write( *mpStrm );

        mpPptEscherEx->WriteDrawingGroupContainer( *mpStrm );
        ImplMasterSlideListContainer( mpStrm );
        ImplDocumentListContainer( mpStrm );

        sal_uInt32 nOldPos = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_CurrentPos );
        if ( nOldPos )
        {
            mpStrm->Seek( nOldPos );
            return TRUE;
        }
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropValue::GetPropertyValue(
    ::com::sun::star::uno::Any& rAny,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
            const String& rString,
                    sal_Bool bTestPropertyAvailability )
{
    sal_Bool bRetValue = sal_True;
    if ( bTestPropertyAvailability )
    {
        bRetValue = sal_False;
        try
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                aXPropSetInfo( rXPropSet->getPropertySetInfo() );
            if ( aXPropSetInfo.is() )
                bRetValue = aXPropSetInfo->hasPropertyByName( rString );
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    if ( bRetValue )
    {
        try
        {
            rAny = rXPropSet->getPropertyValue( rString );
            if ( !rAny.hasValue() )
                bRetValue = sal_False;
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

::com::sun::star::beans::PropertyState PropValue::GetPropertyState(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        const String& rPropertyName )
{
    ::com::sun::star::beans::PropertyState eRetValue = ::com::sun::star::beans::PropertyState_AMBIGUOUS_VALUE;
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState > aXPropState
                ( rXPropSet, ::com::sun::star::uno::UNO_QUERY );
        if ( aXPropState.is() )
            eRetValue = aXPropState->getPropertyState( rPropertyName );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        //...
    }
    return eRetValue;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropValue::ImplGetPropertyValue( const String& rString )
{
    return GetPropertyValue( mAny, mXPropSet, rString );
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropValue::ImplGetPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & aXPropSet, const String& rString )
{
    return GetPropertyValue( mAny, aXPropSet, rString );
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropStateValue::ImplGetPropertyValue( const String& rString, sal_Bool bGetPropertyState )
{
    ePropState = ::com::sun::star::beans::PropertyState_AMBIGUOUS_VALUE;
    sal_Bool bRetValue = TRUE;
#ifdef UNX
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
            aXPropSetInfo( mXPropSet->getPropertySetInfo() );
        if ( !aXPropSetInfo.is() )
            return sal_False;
#endif
    try
    {
        mAny = mXPropSet->getPropertyValue( rString );
        if ( !mAny.hasValue() )
            bRetValue = FALSE;
        else if ( bGetPropertyState )
            ePropState = mXPropState->getPropertyState( rString );
        else
            ePropState = ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        bRetValue = FALSE;
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplInitSOIface()
{
    while( TRUE )
    {
        mXDrawPagesSupplier = ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPagesSupplier >
                ( mXModel, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXDrawPagesSupplier.is() )
            break;

        mXMasterPagesSupplier = ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XMasterPagesSupplier >
                ( mXModel, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXMasterPagesSupplier.is() )
            break;
        mXDrawPages = mXMasterPagesSupplier->getMasterPages();
        if ( !mXDrawPages.is() )
            break;
        mnMasterPages = mXDrawPages->getCount();
        mXDrawPages = mXDrawPagesSupplier->getDrawPages();
        if( !mXDrawPages.is() )
            break;
        mnPages =  mXDrawPages->getCount();
        if ( !ImplGetPageByIndex( 0, NORMAL ) )
            break;

        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetPageByIndex( sal_uInt32 nIndex, PageType ePageType )
{
    while( TRUE )
    {
        if ( ePageType != meLatestPageType )
        {
            switch( ePageType )
            {
                case NORMAL :
                case NOTICE :
                {
                    mXDrawPages = mXDrawPagesSupplier->getDrawPages();
                    if( !mXDrawPages.is() )
                        return FALSE;
                }
                break;

                case MASTER :
                {
                    mXDrawPages = mXMasterPagesSupplier->getMasterPages();
                    if( !mXDrawPages.is() )
                        return FALSE;
                }
                break;
            }
            meLatestPageType = ePageType;
        }
        ::com::sun::star::uno::Any aAny( mXDrawPages->getByIndex( nIndex ) );
        aAny >>= mXDrawPage;
        if ( !mXDrawPage.is() )
            break;
        if ( ePageType == NOTICE )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentationPage >
                aXPresentationPage( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
            if ( !aXPresentationPage.is() )
                break;
            mXDrawPage = aXPresentationPage->getNotesPage();
            if ( !mXDrawPage.is() )
                break;
        }
        mXPagePropSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXPagePropSet.is() )
            break;

        mXShapes = ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShapes >
                ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXShapes.is() )
            break;
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetShapeByIndex( sal_uInt32 nIndex, sal_Bool bGroup )
{
    while(TRUE)
    {
        if (  ( bGroup == FALSE ) || ( GetCurrentGroupLevel() == 0 ) )
        {
            ::com::sun::star::uno::Any aAny( mXShapes->getByIndex( nIndex ) );
            aAny >>= mXShape;
        }
        else
        {
            ::com::sun::star::uno::Any aAny( GetCurrentGroupAccess()->getByIndex( GetCurrentGroupIndex() ) );
            aAny >>= mXShape;
        }
        if ( !mXShape.is() )
            break;

        ::com::sun::star::uno::Any aAny( mXShape->queryInterface( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >*) 0 ) ));
        aAny >>= mXPropSet;

        if ( !mXPropSet.is() )
            break;
        maPosition = ImplMapPoint( mXShape->getPosition() );
        maSize = ImplMapSize( mXShape->getSize() );
        maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
        mType = ByteString( String( mXShape->getShapeType() ), RTL_TEXTENCODING_UTF8 );
        mType.Erase( 0, 13 );                                   // "com.sun.star." entfernen
        sal_uInt16 nPos = mType.Search( (const char*)"Shape" );
        mType.Erase( nPos, 5 );

        mbPresObj = mbEmptyPresObj = FALSE;
        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsPresentationObject" ) ) ) )
            mAny >>= mbPresObj;

        if ( mbPresObj && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsEmptyPresentationObject" ) ) ) )
            mAny >>= mbEmptyPresObj;

        mnAngle = ( PropValue::GetPropertyValue( aAny,
            mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ), sal_True ) )
                ? *((sal_Int32*)aAny.getValue() )
                : 0;

        return TRUE;
    }
    return FALSE;
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteTextBundle( EscherPropertyContainer& rPropOpt, sal_Bool bDisableAutoGrowHeight, sal_Bool bWriteEvenEmptyTextObjects )
{
    if ( ImplGetText() || bWriteEvenEmptyTextObjects )
    {
        ::com::sun::star::uno::Any aAny;
        ::com::sun::star::text::WritingMode             eWM( ::com::sun::star::text::WritingMode_LR_TB );
        ::com::sun::star::drawing::TextVerticalAdjust   eVA( ::com::sun::star::drawing::TextVerticalAdjust_TOP );
        ::com::sun::star::drawing::TextHorizontalAdjust eHA( ::com::sun::star::drawing::TextHorizontalAdjust_LEFT );

        sal_Int32 nLeft             ( 0 );
        sal_Int32 nTop              ( 0 );
        sal_Int32 nRight            ( 0 );
        sal_Int32 nBottom           ( 0 );
        sal_Bool bAutoGrowWidth     ( sal_False );
        sal_Bool bAutoGrowHeight    ( sal_False );

        if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextWritingMode" ) ), sal_True ) )
            aAny >>= eWM;
        if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextVerticalAdjust" ) ), sal_True ) )
            aAny >>= eVA;
        if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextHorizontalAdjust" ) ), sal_True ) )
            aAny >>= eHA;
        if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowWidth" ) ), sal_True ) )
            aAny >>= bAutoGrowWidth;
        if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowHeight" ) ), sal_True ) )
            aAny >>= bAutoGrowHeight;
        if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextLeftDistance" ) ) ) )
            aAny >>= nLeft;
        if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextUpperDistance" ) ) ) )
            aAny >>= nTop;
        if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextRightDistance" ) ) ) )
            aAny >>= nRight;
        if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextLowerDistance" ) ) ) )
            aAny >>= nBottom;


        ESCHER_AnchorText eAnchor = ESCHER_AnchorTop;
        ESCHER_WrapMode eWrapMode = ESCHER_WrapSquare;
        sal_uInt32 nTextAttr = 0x40004;     // rotate text with shape

        if ( eWM == ::com::sun::star::text::WritingMode_TB_RL )
        {   // verical writing
            switch ( eHA )
            {
                case ::com::sun::star::drawing::TextHorizontalAdjust_LEFT :
                     eAnchor = ESCHER_AnchorBottom;
                break;
                case ::com::sun::star::drawing::TextVerticalAdjust_CENTER :
                    eAnchor = ESCHER_AnchorMiddle;
                break;
                default :
                case ::com::sun::star::drawing::TextHorizontalAdjust_BLOCK :
                case ::com::sun::star::drawing::TextHorizontalAdjust_RIGHT :
                    eAnchor = ESCHER_AnchorTop;
                break;
            }
            if ( eVA == ::com::sun::star::drawing::TextVerticalAdjust_CENTER )
            {
                switch ( eAnchor )
                {
                    case ESCHER_AnchorMiddle :
                        eAnchor = ESCHER_AnchorMiddleCentered;
                    break;
                    case ESCHER_AnchorBottom :
                        eAnchor = ESCHER_AnchorBottomCentered;
                    break;
                    default :
                    case ESCHER_AnchorTop :
                        eAnchor = ESCHER_AnchorTopCentered;
                    break;
                }
            }
            if ( bAutoGrowHeight )
                eWrapMode = ESCHER_WrapNone;
            if ( !bDisableAutoGrowHeight && bAutoGrowWidth )
                nTextAttr |= 0x20002;

             rPropOpt.AddOpt( ESCHER_Prop_txflTextFlow, ESCHER_txflTtoBA ); // rotate text within shape by 90
        }
        else
        {   // normal from left to right
            switch ( eVA )
            {
                case ::com::sun::star::drawing::TextVerticalAdjust_CENTER :
                    eAnchor = ESCHER_AnchorMiddle;
                break;

                case ::com::sun::star::drawing::TextVerticalAdjust_BOTTOM :
                    eAnchor = ESCHER_AnchorBottom;
                break;

                default :
                case ::com::sun::star::drawing::TextVerticalAdjust_TOP :
                    eAnchor = ESCHER_AnchorTop;
                break;
            }
            if ( eHA == ::com::sun::star::drawing::TextHorizontalAdjust_CENTER )
            {
                switch( eAnchor )
                {
                    case ESCHER_AnchorMiddle :
                        eAnchor = ESCHER_AnchorMiddleCentered;
                    break;
                    case ESCHER_AnchorBottom :
                        eAnchor = ESCHER_AnchorBottomCentered;
                    break;
                    case ESCHER_AnchorTop :
                        eAnchor = ESCHER_AnchorTopCentered;
                    break;
                }
            }
            if ( bAutoGrowWidth )
                eWrapMode = ESCHER_WrapNone;
            if ( !bDisableAutoGrowHeight && bAutoGrowHeight )
                nTextAttr |= 0x20002;
        }
        rPropOpt.AddOpt( ESCHER_Prop_dxTextLeft, nLeft * 360 );
        rPropOpt.AddOpt( ESCHER_Prop_dxTextRight, nRight * 360 );
        rPropOpt.AddOpt( ESCHER_Prop_dyTextTop, nTop * 360 );
        rPropOpt.AddOpt( ESCHER_Prop_dyTextBottom, nBottom * 360 );

        rPropOpt.AddOpt( ESCHER_Prop_WrapText, eWrapMode );
        rPropOpt.AddOpt( ESCHER_Prop_AnchorText, eAnchor );
        rPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, nTextAttr );
        rPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
    }
}

//  -----------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplGetMasterIndex( PageType ePageType )
{
    sal_uInt32 nRetValue = 0;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XMasterPageTarget >
        aXMasterPageTarget( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

    if ( aXMasterPageTarget.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
            aXDrawPage = aXMasterPageTarget->getMasterPage();
        if ( aXDrawPage.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                aXPropertySet( aXDrawPage, ::com::sun::star::uno::UNO_QUERY );

            if ( aXPropertySet.is() )
            {
                if ( ImplGetPropertyValue( aXPropertySet, String( RTL_CONSTASCII_USTRINGPARAM( "Number" ) ) ) )
                    nRetValue |= *(sal_Int16*)mAny.getValue();
                if ( nRetValue & 0xffff )           // ueberlauf vermeiden
                    nRetValue--;
            }
        }
    }
    if ( ePageType == NOTICE )
        nRetValue += mnMasterPages;
    return nRetValue;
}

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetStyleSheets()
{
    int             nInstance, nLevel;
    sal_Bool        bRetValue;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >
        aXNamed;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
        aXNameAccess;

    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyleFamiliesSupplier >
        aXStyleFamiliesSupplier( mXModel, ::com::sun::star::uno::UNO_QUERY );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        aXPropSet( mXModel, ::com::sun::star::uno::UNO_QUERY );

    sal_uInt16 nDefaultTab = ( aXPropSet.is() && ImplGetPropertyValue( aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TabStop" ) ) ) )
        ? (sal_uInt16)( *(sal_Int32*)mAny.getValue() / 4.40972 )
        : 1250;

    mpStyleSheet = new PPTExStyleSheet( nDefaultTab, (PPTExBulletProvider&)*this );

    if ( ImplGetPageByIndex( 0, MASTER ) )
        aXNamed = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >
                    ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

    if ( aXStyleFamiliesSupplier.is() )
        aXNameAccess = aXStyleFamiliesSupplier->getStyleFamilies();

    bRetValue = aXNamed.is() && aXNameAccess.is() && aXStyleFamiliesSupplier.is();
    if  ( bRetValue )
    {
        for ( nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_Other; nInstance++ )
        {
            String aStyle;
            String aFamily;
            switch ( nInstance )
            {
                case EPP_TEXTTYPE_Title :
                {
                    aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "title" ) );
                    aFamily = aXNamed->getName();
                }
                break;
                case EPP_TEXTTYPE_Body :
                {
                    aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "outline1" ) );      // SD_LT_SEPARATOR
                    aFamily = aXNamed->getName();
                }
                break;
                case EPP_TEXTTYPE_Other :
                {
                    aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "standard" ) );
                    aFamily = String( RTL_CONSTASCII_USTRINGPARAM( "graphics" ) );
                }
                break;
            }
            if ( aStyle.Len() && aFamily.Len() )
            {
                try
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >xNameAccess;
                    if ( aXNameAccess->hasByName( aFamily ) )
                    {
                        ::com::sun::star::uno::Any aAny( aXNameAccess->getByName( aFamily ) );
                        if( aAny.getValue() && ::cppu::extractInterface( xNameAccess, aAny ) )
                        {
                            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > aXFamily;
                            if ( aAny >>= aXFamily )
                            {
                                if ( aXFamily->hasByName( aStyle ) )
                                {
                                    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > xStyle;
                                    aAny = aXFamily->getByName( aStyle );
                                    if( aAny.getValue() && ::cppu::extractInterface( xStyle, aAny ) )
                                    {
                                        ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > aXStyle;
                                        aAny >>= aXStyle;
                                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                            xPropSet( aXStyle, ::com::sun::star::uno::UNO_QUERY );
                                        if( xPropSet.is() )
                                            mpStyleSheet->SetStyleSheet( xPropSet, maFontCollection, nInstance, 0 );
                                        if ( nInstance == EPP_TEXTTYPE_Body )
                                        {
                                            for ( nLevel = 1; nLevel < 5; nLevel++ )
                                            {
                                                sal_Unicode cTemp = aStyle.GetChar( aStyle.Len() - 1 );
                                                aStyle.SetChar( aStyle.Len() - 1, ++cTemp );
                                                if ( aXFamily->hasByName( aStyle ) )
                                                {
                                                    aAny = aXFamily->getByName( aStyle );
                                                    if( aAny.getValue() && ::cppu::extractInterface( xStyle, aAny ) )
                                                    {
                                                        ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > aXStyle;
                                                        aAny >>= aXStyle;
                                                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                                            xPropSet( aXStyle, ::com::sun::star::uno::UNO_QUERY );
                                                        if ( xPropSet.is() )
                                                            mpStyleSheet->SetStyleSheet( xPropSet, maFontCollection, nInstance, nLevel );
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                catch( ::com::sun::star::uno::Exception& )
                {
                //
                }
            }
        }
        for ( ; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
        {

        }
    }
    return bRetValue;
}

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetMasterTitleAndBody()
{
    sal_uInt32  i, nSearchFor, nShapes;

    if ( !ImplGetPageByIndex( 0, MASTER ) )
        return FALSE;

    nShapes = mXShapes->getCount();

    // ueber die Seite gehen um den Index des Title and Body Objects zu bekommen
    for ( nSearchFor = 2, i = 0; i < nShapes; i++ )
    {
        if ( !ImplGetShapeByIndex( i ) )
            break;

        if ( mType == "presentation.TitleText" )
        {
            mnMasterTitleIndex = i;
            if ( ! ( --nSearchFor ) )
                break;
        }
        else if ( ( mType == "presentation.Outliner" ) || ( mType == "presentation.Subtitle" ) )
        {
            mnMasterBodyIndex = i;
            if ( ! ( --nSearchFor ) )
                break;
        }
    };
    return TRUE;
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteParagraphs( SvStream& rOut, TextObj& rTextObj )
{
    sal_Bool            bFirstParagraph = TRUE;
    sal_uInt32          nCharCount;
    sal_uInt32          nPropertyFlags = 0;
    sal_uInt16          nDepth = 0;
    sal_Int16           nLineSpacing;
    int             nInstance = rTextObj.GetInstance();

    for ( ParagraphObj* pPara = rTextObj.First() ; pPara; pPara = rTextObj.Next(), bFirstParagraph = FALSE )
    {
        nCharCount = pPara->Count();

        nDepth = pPara->nDepth;
        if ( nDepth > 4)
            nDepth = 4;

        if ( ( pPara->meTextAdjust == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, ParaAttr_Adjust, pPara->mnTextAdjust ) ) )
            nPropertyFlags |= 0x00000800;
        nLineSpacing = pPara->mnLineSpacing;
        if ( bFirstParagraph && ( nLineSpacing > 100 ) )
        {
            nLineSpacing = 100;
            nPropertyFlags |= 0x00001000;
        }
        else if ( ( pPara->meLineSpacing == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, ParaAttr_LineFeed, nLineSpacing ) ) )
            nPropertyFlags |= 0x00001000;
        if ( ( pPara->meLineSpacingBottom == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, ParaAttr_LowerDist, pPara->mnLineSpacingBottom ) ) )
            nPropertyFlags |= 0x00004000;
        if ( ( pPara->meLineSpacingTop == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, ParaAttr_UpperDist, pPara->mnLineSpacingTop ) ) )
            nPropertyFlags |= 0x00002000;

        sal_Int32 nBuRealSize = pPara->nBulletRealSize;
        sal_Int16 nBulletFlags = pPara->nBulletFlags;

        if ( pPara->bExtendedParameters )
            nPropertyFlags |= pPara->nParaFlags;
        else
        {
            nPropertyFlags |= 1;            // turn off bullet explicit
            nBulletFlags = 0;
        }
/*
        PortionObj* pPortion = (PortionObj*)pPara->First();
        if ( pPortion ) // in SO the bulletrealsize does not depend to the following portion charactersize
        {
            if ( pPortion->mnCharHeight )
            {
                if ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_FontHeight, pPortion->mnCharHeight ) )
                {   // the fontsize is unequal to the StyleSheetFontSize, so the BulletRealSize has to be set again
                    nPropertyFlags |= 0x44;
                    nBulletFlags |= 8;
                    nBuRealSize *= mpStyleSheet->GetCharSheet( nInstance ).maCharLevel[ pPara->nDepth ].mnFontHeight;
                    nBuRealSize /= pPortion->mnCharHeight;
                }
            }
        }
*/
        rOut << nCharCount
             << nDepth                          // Level
             << (sal_uInt32)nPropertyFlags;     // Paragraph Attribut Set

        if ( nPropertyFlags & 0xf )
            rOut << nBulletFlags;
        if ( nPropertyFlags & 0x80 )
            rOut << (sal_uInt16)( pPara->cBulletId );
        if ( nPropertyFlags & 0x10 )
        {
            FontCollectionEntry aFontDescEntry( pPara->aFontDesc.Name, pPara->aFontDesc.Family, pPara->aFontDesc.Pitch, pPara->aFontDesc.CharSet );
            rOut << (sal_uInt16)( maFontCollection.GetId( aFontDescEntry ) );
        }
        if ( nPropertyFlags & 0x40 )
            rOut << (sal_Int16)nBuRealSize;
        if ( nPropertyFlags & 0x20 )
            rOut << pPara->nBulletColor;
        if ( nPropertyFlags & 0x00000800 )
            rOut << (sal_uInt16)( pPara->mnTextAdjust );
        if ( nPropertyFlags & 0x00001000 )
            rOut << (sal_uInt16)( nLineSpacing );
        if ( nPropertyFlags & 0x00002000 )
            rOut << (sal_uInt16)( pPara->mnLineSpacingTop );
        if ( nPropertyFlags & 0x00004000 )
            rOut << (sal_uInt16)( pPara->mnLineSpacingBottom );
    }
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWritePortions( SvStream& rOut, TextObj& rTextObj )
{
    sal_uInt32  nPropertyFlags, i = 0;
    int     nInstance = rTextObj.GetInstance();

    for ( ParagraphObj* pPara = rTextObj.First(); pPara; pPara = rTextObj.Next(), i++ )
    {
        for ( PortionObj* pPortion = (PortionObj*)pPara->First(); pPortion; pPortion = (PortionObj*)pPara->Next() )
        {
            nPropertyFlags = 0;
            sal_uInt32 nCharAttr = pPortion->mnCharAttr;

            if ( nInstance == 4 )                       // special handling for normal textobjects:
                nPropertyFlags |= nCharAttr & 0x17;     // not all attributes ar inherited
            else
            {
                if ( ( pPortion->mnCharAttrHard & 1 ) ||
                    ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Bold, nCharAttr ) ) )
                    nPropertyFlags |= 1;
                if ( ( pPortion->mnCharAttrHard & 2 ) ||
                    ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Italic, nCharAttr ) ) )
                    nPropertyFlags |= 2;
                if ( ( pPortion->mnCharAttrHard & 4 ) ||
                    ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Underline, nCharAttr ) ) )
                    nPropertyFlags |= 4;
                if ( ( pPortion->mnCharAttrHard & 0x10 ) ||
                    ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Shadow, nCharAttr ) ) )
                    nPropertyFlags |= 0x10;
            }
            if ( rTextObj.HasExtendedBullets() )
            {
                if ( i > 63 )
                    i = 63;

                nPropertyFlags |= i << 10 ;
                nCharAttr  |= i << 10;
            }
            if ( ( pPortion->meFontName == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Font, pPortion->mnFont ) ) )
                nPropertyFlags |= 0x00010000;
            if ( ( pPortion->meAsianOrComplexFont == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_AsianOrComplexFont, pPortion->mnAsianOrComplexFont ) ) )
                nPropertyFlags |= 0x00200000;
            if ( ( pPortion->meCharHeight == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_FontHeight, pPortion->mnCharHeight ) ) )
                nPropertyFlags |= 0x00020000;
            if ( ( pPortion->meCharColor == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_FontColor, pPortion->mnCharColor ) ) )
                nPropertyFlags |= 0x00040000;
            if ( ( pPortion->meCharEscapement == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Escapement, pPortion->mnCharEscapement ) ) )
                nPropertyFlags |= 0x00080000;

            sal_uInt32 nCharCount = pPortion->Count();

            rOut << nCharCount
                 << nPropertyFlags;          //PropertyFlags

            if ( nPropertyFlags & 0xffff )
                rOut << (sal_uInt16)( nCharAttr );
            if ( nPropertyFlags & 0x00010000 )
                rOut << pPortion->mnFont;
            if ( nPropertyFlags & 0x00200000 )
                rOut << pPortion->mnAsianOrComplexFont;
            if ( nPropertyFlags & 0x00020000 )
                rOut << (sal_uInt16)( pPortion->mnCharHeight );
            if ( nPropertyFlags & 0x00040000 )
                rOut << (sal_uInt32)pPortion->mnCharColor;
            if ( nPropertyFlags & 0x00080000 )
                rOut << pPortion->mnCharEscapement;
        }
    }
}

//  ----------------------------------------------------------------------------------------
//  laedt und konvertiert text aus shape, ergebnis ist mnTextSize gespeichert;
sal_Bool PPTWriter::ImplGetText()
{
    mnTextSize = 0;
    mXText = ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XSimpleText >
            ( mXShape, ::com::sun::star::uno::UNO_QUERY );

    if ( mXText.is() )
        mnTextSize = mXText->getString().len();
    return ( mnTextSize != 0 );
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplFlipBoundingBox( EscherPropertyContainer& rPropOpt )
{
    if ( mnAngle < 0 )
        mnAngle = ( 36000 + mnAngle ) % 36000;
    else
        mnAngle = ( 36000 - ( mnAngle % 36000 ) );

    double  fCos = cos( (double)mnAngle * F_PI18000 );
    double  fSin = sin( (double)mnAngle * F_PI18000 );

    double  fWidthHalf = maRect.GetWidth() / 2;
    double  fHeightHalf = maRect.GetHeight() / 2;

    double  fXDiff = fCos * fWidthHalf + fSin * (-fHeightHalf);
    double  fYDiff = - ( fSin * fWidthHalf - fCos * ( -fHeightHalf ) );

    maRect.Move( (sal_Int32)( -( fWidthHalf - fXDiff ) ), (sal_Int32)(  - ( fHeightHalf + fYDiff ) ) );

    if ( ( mnAngle > 4500 && mnAngle <= 13500 ) || ( mnAngle > 22500 && mnAngle <= 31500 ) )
    {
        // In diesen beiden Bereichen steht in PPT gemeinerweise die
        // BoundingBox bereits senkrecht. Daher muss diese VOR
        // DER ROTATION flachgelegt werden.
        ::com::sun::star::awt::Point
            aTopLeft( (sal_Int32)( maRect.Left() + fWidthHalf - fHeightHalf ), (sal_Int32)( maRect.Top() + fHeightHalf - fWidthHalf ) );
        Size    aNewSize( maRect.GetHeight(), maRect.GetWidth() );
        maRect = Rectangle( Point( aTopLeft.X, aTopLeft.Y ), aNewSize );
    }
    mnAngle *= 655;
    mnAngle += 0x8000;
    mnAngle &=~0xffff;                                  // nAngle auf volle Gradzahl runden
    rPropOpt.AddOpt( ESCHER_Prop_Rotation, mnAngle );
}

//  -----------------------------------------------------------------------

struct FieldEntry
{
    sal_uInt32  nFieldType;
    sal_uInt32  nFieldStartPos;
    sal_uInt32  nFieldEndPos;
    String      aFieldUrl;

    FieldEntry( sal_uInt32 nType, sal_uInt32 nStart, sal_uInt32 nEnd )
    {
        nFieldType = nType;
        nFieldStartPos = nStart;
        nFieldEndPos = nEnd;
    }
    FieldEntry( FieldEntry& rFieldEntry )
    {
        nFieldType = rFieldEntry.nFieldType;
        nFieldStartPos = rFieldEntry.nFieldStartPos;
        nFieldEndPos = rFieldEntry.nFieldEndPos;
        aFieldUrl = rFieldEntry.aFieldUrl;
    }
};

//  -----------------------------------------------------------------------

PortionObj::PortionObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                FontCollection& rFontCollection ) :
    mbLastPortion       ( TRUE ),
    mnCharAttrHard      ( 0 ),
    mnCharAttr          ( 0 ),
    mnTextSize          ( 0 ),
    mnFont              ( 0 ),
    mnAsianOrComplexFont( 0xffff ),
    mpFieldEntry        ( NULL ),
    mpText              ( NULL )
{
    mXPropSet = rXPropSet;

    ImplGetPortionValues( rFontCollection, FALSE );
}

PortionObj::PortionObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & rXTextRange,
                            sal_Bool bLast, FontCollection& rFontCollection ) :
    mbLastPortion           ( bLast ),
    mnCharAttrHard          ( 0 ),
    mnCharAttr              ( 0 ),
    mnFont                  ( 0 ),
    mnAsianOrComplexFont    ( 0xffff ),
    mpFieldEntry            ( NULL ),
    mpText                  ( NULL )
{
    String aString( rXTextRange->getString() );
    String aURL;

    mnTextSize = aString.Len();
    if ( bLast )
        mnTextSize++;

    if ( mnTextSize )
    {
        mpFieldEntry = NULL;
        sal_uInt32 nFieldType = 0;

        mXPropSet = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >
                ( rXTextRange, ::com::sun::star::uno::UNO_QUERY );
        mXPropState = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyState >
                ( rXTextRange, ::com::sun::star::uno::UNO_QUERY );

        sal_Bool bPropSetsValid = ( mXPropSet.is() && mXPropState.is() );
        if ( bPropSetsValid )
            nFieldType = ImplGetTextField( rXTextRange, aURL );
        if ( nFieldType )
        {
            mpFieldEntry = new FieldEntry( nFieldType, 0, mnTextSize );
            if ( ( nFieldType >> 28 == 4 ) )
                mpFieldEntry->aFieldUrl = aString;
        }

        sal_Bool bSymbol = FALSE;

        if ( bPropSetsValid && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSet" ) ), FALSE ) )
        {
            sal_Int16 nCharset;
            mAny >>= nCharset;
            if ( nCharset == ::com::sun::star::awt::CharSet::SYMBOL )
                bSymbol = TRUE;
        }
        if ( mpFieldEntry && ( nFieldType & 0x800000 ) )    // placeholder ?
        {
            mnTextSize = 1;
            if ( bLast )
                mnTextSize++;
            mpText = new sal_uInt16[ mnTextSize ];
            mpText[ 0 ] = 0x2a;
        }
        else
        {
            mpText = new sal_uInt16[ mnTextSize ];
            const sal_Unicode* pText = aString.GetBuffer();
            sal_uInt16 nChar;
            for ( int i = 0; i < aString.Len(); i++ )
            {
                nChar = (sal_uInt16)pText[ i ];
                if ( nChar == 0xa )
                    nChar++;
                else if ( !bSymbol )
                {
                    switch ( nChar )
                    {
                        // Currency
                        case 128:   nChar = 0x20AC; break;
                        // Punctuation and other
                        case 130:   nChar = 0x201A; break;// SINGLE LOW-9 QUOTATION MARK
                        case 131:   nChar = 0x0192; break;// LATIN SMALL LETTER F WITH HOOK
                        case 132:   nChar = 0x201E; break;// DOUBLE LOW-9 QUOTATION MARK
                                                              // LOW DOUBLE PRIME QUOTATION MARK
                        case 133:   nChar = 0x2026; break;// HORIZONTAL ELLIPSES
                        case 134:   nChar = 0x2020; break;// DAGGER
                        case 135:   nChar = 0x2021; break;// DOUBLE DAGGER
                        case 136:   nChar = 0x02C6; break;// MODIFIER LETTER CIRCUMFLEX ACCENT
                        case 137:   nChar = 0x2030; break;// PER MILLE SIGN
                        case 138:   nChar = 0x0160; break;// LATIN CAPITAL LETTER S WITH CARON
                        case 139:   nChar = 0x2039; break;// SINGLE LEFT-POINTING ANGLE QUOTATION MARK
                        case 140:   nChar = 0x0152; break;// LATIN CAPITAL LIGATURE OE
                        case 142:   nChar = 0x017D; break;// LATIN CAPITAL LETTER Z WITH CARON
                        case 145:   nChar = 0x2018; break;// LEFT SINGLE QUOTATION MARK
                                                              // MODIFIER LETTER TURNED COMMA
                        case 146:   nChar = 0x2019; break;// RIGHT SINGLE QUOTATION MARK
                                                              // MODIFIER LETTER APOSTROPHE
                        case 147:   nChar = 0x201C; break;// LEFT DOUBLE QUOTATION MARK
                                                              // REVERSED DOUBLE PRIME QUOTATION MARK
                        case 148:   nChar = 0x201D; break;// RIGHT DOUBLE QUOTATION MARK
                                                              // REVERSED DOUBLE PRIME QUOTATION MARK
                        case 149:   nChar = 0x2022; break;// BULLET
                        case 150:   nChar = 0x2013; break;// EN DASH
                        case 151:   nChar = 0x2014; break;// EM DASH
                        case 152:   nChar = 0x02DC; break;// SMALL TILDE
                        case 153:   nChar = 0x2122; break;// TRADE MARK SIGN
                        case 154:   nChar = 0x0161; break;// LATIN SMALL LETTER S WITH CARON
                        case 155:   nChar = 0x203A; break;// SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
                        case 156:   nChar = 0x0153; break;// LATIN SMALL LIGATURE OE
                        case 158:   nChar = 0x017E; break;// LATIN SMALL LETTER Z WITH CARON
                        case 159:   nChar = 0x0178; break;// LATIN CAPITAL LETTER Y WITH DIAERESIS
//                      case 222:   nChar = 0x00B6; break;// PILCROW SIGN / PARAGRAPH SIGN
                    }
                }
                mpText[ i ] = nChar;
            }
        }
        if ( bLast )
            mpText[ mnTextSize - 1 ] = 0xd;

        if ( bPropSetsValid )
            ImplGetPortionValues( rFontCollection, TRUE );
    }
}

PortionObj::PortionObj( PortionObj& rPortionObj )
{
    ImplConstruct( rPortionObj );
}

PortionObj::~PortionObj()
{
    ImplClear();
}

void PortionObj::Write( SvStream* pStrm, sal_Bool bLast )
{
    sal_uInt32 nCount = mnTextSize;
    if ( bLast && mbLastPortion )
        nCount--;
    for ( sal_uInt32 i = 0; i < nCount; i++ )
        *pStrm << (sal_uInt16)mpText[ i ];
}

void PortionObj::ImplGetPortionValues( FontCollection& rFontCollection, sal_Bool bGetPropStateValue )
{

    sal_Bool bOk = ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontName" ) ), bGetPropStateValue );
    meFontName = ePropState;
    if ( bOk )
    {
        FontCollectionEntry aFontDesc( *(::rtl::OUString*)mAny.getValue() );
        sal_uInt32  nCount = rFontCollection.GetCount();
        mnFont = (sal_uInt16)rFontCollection.GetId( aFontDesc );
        if ( mnFont == nCount )
        {
            FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSet" ) ), sal_False ) )
                mAny >>= rFontDesc.CharSet;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontFamily" ) ), sal_False ) )
                mAny >>= rFontDesc.Family;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontPitch" ) ), sal_False ) )
                mAny >>= rFontDesc.Pitch;
        }
    }
    bOk = ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontNameAsian" ) ), bGetPropStateValue );
    meAsianOrComplexFont = ePropState;
    if ( bOk )
    {
        FontCollectionEntry aFontDesc( *(::rtl::OUString*)mAny.getValue() );
        sal_uInt32  nCount = rFontCollection.GetCount();
        mnAsianOrComplexFont = (sal_uInt16)rFontCollection.GetId( aFontDesc );
        if ( mnAsianOrComplexFont == nCount )
        {
            FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSetAsian" ) ), sal_False ) )
                mAny >>= rFontDesc.CharSet;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontFamilyAsian" ) ), sal_False ) )
                mAny >>= rFontDesc.Family;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontPitchAsian" ) ), sal_False ) )
                mAny >>= rFontDesc.Pitch;
        }
    }

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharWeight" ) ), bGetPropStateValue ) )
    {
        float fFloat;
        mAny >>= fFloat;
        if ( fFloat >= ::com::sun::star::awt::FontWeight::SEMIBOLD )
            mnCharAttr |= 1;
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 1;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharPosture" ) ), bGetPropStateValue ) )
    {
        ::com::sun::star::awt::FontSlant aFS;
        mAny >>= aFS;
        switch ( aFS )
        {
            case ::com::sun::star::awt::FontSlant_OBLIQUE :
            case ::com::sun::star::awt::FontSlant_ITALIC :
                mnCharAttr |= 2;
        }
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 2;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharUnderline" ) ), bGetPropStateValue ) )
    {
        sal_Int16 nVal;
        mAny >>= nVal;
        switch ( nVal )
        {
            case ::com::sun::star::awt::FontUnderline::SINGLE :
            case ::com::sun::star::awt::FontUnderline::DOUBLE :
            case ::com::sun::star::awt::FontUnderline::DOTTED :
                mnCharAttr |= 4;
        }
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 4;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharShadowed" ) ), bGetPropStateValue ) )
    {
        sal_Bool bBool;
        mAny >>= bBool;
        if ( bBool )
            mnCharAttr |= 0x10;
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 16;

    mnCharHeight = 24;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" ) ), bGetPropStateValue ) )
    {
        float fVal;
        mAny >>= fVal;
        mnCharHeight = (sal_uInt16)( fVal + 0.5 );
    }
    meCharHeight = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharColor" ) ), bGetPropStateValue ) )
    {
        sal_uInt32 nSOColor = *( (sal_uInt32*)mAny.getValue() );
        mnCharColor = nSOColor & 0xff00;                                // green
        mnCharColor |= (sal_uInt8)( nSOColor ) << 16;                   // red
        mnCharColor |= (sal_uInt8)( nSOColor >> 16 ) | 0xfe000000;      // blue
    }
    meCharColor = ePropState;

    mnCharEscapement = 0;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharEscapement" ) ), bGetPropStateValue ) )
    {
        mAny >>= mnCharEscapement;
        if ( mnCharEscapement > 100 )
            mnCharEscapement = 33;
        else if ( mnCharEscapement < -100 )
            mnCharEscapement = -33;
    }
    meCharEscapement = ePropState;
}

void PortionObj::ImplClear()
{
    delete (FieldEntry*)mpFieldEntry;
    delete mpText;
}

void PortionObj::ImplConstruct( PortionObj& rPortionObj )
{
    mbLastPortion = rPortionObj.mbLastPortion;
    mnTextSize = rPortionObj.mnTextSize;
    mnCharColor = rPortionObj.mnCharColor;
    mnCharEscapement = rPortionObj.mnCharEscapement;
    mnCharAttr = rPortionObj.mnCharAttr;
    mnCharHeight = rPortionObj.mnCharHeight;
    mnFont = rPortionObj.mnFont;
    mnAsianOrComplexFont = rPortionObj.mnAsianOrComplexFont;

    if ( rPortionObj.mpText )
    {
        mpText = new sal_uInt16[ mnTextSize ];
        memcpy( mpText, rPortionObj.mpText, mnTextSize << 1 );
    }
    else
        mpText = NULL;

    if ( rPortionObj.mpFieldEntry )
        mpFieldEntry = new FieldEntry( *( rPortionObj.mpFieldEntry ) );
    else
        mpFieldEntry = NULL;
}

sal_uInt32 PortionObj::ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition )
{
    if ( mpFieldEntry && ( !mpFieldEntry->nFieldStartPos ) )
    {
        mpFieldEntry->nFieldStartPos += nCurrentTextPosition;
        mpFieldEntry->nFieldEndPos += nCurrentTextPosition;
    }
    return mnTextSize;
}

//  -----------------------------------------------------------------------
// Rueckgabe:                           0 = kein TextField
//  bit28->31   art des TextFields :
//                                      1 = Date
//                                      2 = Time
//                                      3 = SlideNumber
//                                      4 = Url
//  bit24->27   PPT Textfield type
//     23->     PPT Textfield needs a placeholder

sal_uInt32 PortionObj::ImplGetTextField( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >
                                            & rXCursorText, String& rURL )
{
    sal_uInt32 nRetValue = 0;
    sal_Int32 nFormat;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField >
        aXTextField( rXCursorText, ::com::sun::star::uno::UNO_QUERY );

    if ( aXTextField.is() )
    {
        mXPropSet = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >
                ( aXTextField, ::com::sun::star::uno::UNO_QUERY );

        if ( mXPropSet.is() )
        {
            String aFieldKind( aXTextField->getPresentation( TRUE ) );
            if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Date" ) ) )
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ) )
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( !bBool )  // Fixed DateFields gibt es in PPT nicht
                    {
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Format" ) ) ) )
                        {
                            nFormat = *(sal_Int32*)mAny.getValue();
                            switch ( nFormat )
                            {
                                default:
                                case 5 :
                                case 4 :
                                case 2 : nFormat = 0; break;
                                case 8 :
                                case 9 :
                                case 3 : nFormat = 1; break;
                                case 7 :
                                case 6 : nFormat = 2; break;
                            }
                            nRetValue |= ( ( ( 1 << 4 ) | nFormat ) << 24 ) | 0x800000;
                        }
                    }
                }
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Url" ) ) )
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) ) )
                    rURL = String( *(::rtl::OUString*)mAny.getValue() );
                nRetValue = 4 << 28;
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Page" ) ) )
            {
                nRetValue = 3 << 28 | 0x800000;
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Pages" ) ) )
            {

            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Time" ) ) )
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ) )
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( !bBool )
                    {
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ) )
                        {
                            nFormat = *(sal_Int32*)mAny.getValue();
                            nRetValue |= ( ( ( 2 << 4 ) | nFormat ) << 24 ) | 0x800000;
                        }
                    }
                }
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) )
            {

            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Table" ) ) )
            {

            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "ExtTime" ) ) )
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ) )
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( !bBool )
                    {
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Format" ) ) ) )
                        {
                            nFormat = *(sal_Int32*)mAny.getValue();
                            switch ( nFormat )
                            {
                                default:
                                case 6 :
                                case 7 :
                                case 8 :
                                case 2 : nFormat = 12; break;
                                case 3 : nFormat = 9; break;
                                case 5 :
                                case 4 : nFormat = 10; break;

                            }
                            nRetValue |= ( ( ( 2 << 4 ) | nFormat ) << 24 ) | 0x800000;
                        }
                    }
                }
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "ExtFile" ) ) )
            {

            }
            else if ( aFieldKind ==  String( RTL_CONSTASCII_USTRINGPARAM( "Author" ) ) )
            {

            }
        }
    }
    return nRetValue;
}

PortionObj& PortionObj::operator=( PortionObj& rPortionObj )
{
    if ( this != &rPortionObj )
    {
        ImplClear();
        ImplConstruct( rPortionObj );
    }
    return *this;
}

//  -----------------------------------------------------------------------

ParagraphObj::ParagraphObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                PPTExBulletProvider& rProv ) :
    maMapModeSrc        ( MAP_100TH_MM ),
    maMapModeDest       ( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) )
{
    mXPropSet = rXPropSet;

    bDepth = bExtendedParameters = FALSE;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    ImplGetParagraphValues( rProv, FALSE );
}

    ParagraphObj::ParagraphObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > & rXTextContent,
                    ParaFlags aParaFlags, FontCollection& rFontCollection, PPTExBulletProvider& rProv ) :
    maMapModeSrc        ( MAP_100TH_MM ),
    maMapModeDest       ( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) ),
    mbFirstParagraph    ( aParaFlags.bFirstParagraph ),
    mbLastParagraph     ( aParaFlags.bLastParagraph )
{
    bDepth = bExtendedParameters = FALSE;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    mXPropSet = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            ( rXTextContent, ::com::sun::star::uno::UNO_QUERY );

    mXPropState = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertyState >
            ( rXTextContent, ::com::sun::star::uno::UNO_QUERY );

    if ( mXPropSet.is() && mXPropState.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
            aXTextPortionEA( rXTextContent, ::com::sun::star::uno::UNO_QUERY );
        if ( aXTextPortionEA.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
                aXTextPortionE( aXTextPortionEA->createEnumeration() );
            if ( aXTextPortionE.is() )
            {
                while ( aXTextPortionE->hasMoreElements() )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > aXCursorText;
                    ::com::sun::star::uno::Any aAny( aXTextPortionE->nextElement() );
                    if ( aAny >>= aXCursorText )
                    {
                        PortionObj* pPortionObj = new PortionObj( aXCursorText, !aXTextPortionE->hasMoreElements(), rFontCollection );
                        if ( pPortionObj->Count() )
                            Insert( pPortionObj, LIST_APPEND );
                        else
                            delete pPortionObj;
                    }
                }
            }
        }
        ImplGetParagraphValues( rProv, TRUE );//
    }
}

ParagraphObj::ParagraphObj( ParagraphObj& rObj )
{
    ImplConstruct( rObj );
}

ParagraphObj::~ParagraphObj()
{
    ImplClear();
}

void ParagraphObj::Write( SvStream* pStrm )
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        ((PortionObj*)pPtr)->Write( pStrm, mbLastParagraph );
}

void ParagraphObj::ImplClear()
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        delete (PortionObj*)pPtr;
}

void ParagraphObj::CalculateGraphicBulletSize( sal_uInt16 nFontHeight )
{
    if ( ( (SvxExtNumType)nNumberingType == SVX_NUM_BITMAP ) && ( nBulletId != 0xffff ) )
    {
        // calculate the bulletrealsize for this grafik
        if ( aBuGraSize.Width() && aBuGraSize.Height() )
        {
            double fCharHeight = nFontHeight;
            double fLen = aBuGraSize.Height();
            fCharHeight = fCharHeight * 0.2540;
            double fQuo = fLen / fCharHeight;
            nBulletRealSize = (sal_Int16)( fQuo + 0.5 );
            if ( (sal_uInt16)nBulletRealSize > 400 )
                nBulletRealSize = 400;
        }
    }
}

void ParagraphObj::ImplGetNumberingLevel( PPTExBulletProvider& rBuProv, sal_Int16 nDepth, sal_Bool bGetPropStateValue )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > aXIndexReplace;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ), bGetPropStateValue ) )
    {
        if ( ( mAny >>= aXIndexReplace ) && nDepth < aXIndexReplace->getCount() )
        {
            mAny <<= aXIndexReplace->getByIndex( nDepth );
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                aPropertySequence( *( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>*)mAny.getValue() );

            const ::com::sun::star::beans::PropertyValue* pPropValue = aPropertySequence.getArray();

            sal_Int32 nCount = aPropertySequence.getLength();
            if ( nCount )
            {
                bExtendedParameters = TRUE;
                nBulletRealSize = 100;
                nMappedNumType = 0;

                String aGraphicURL;
                for ( sal_Int32 i = 0; i < nCount; i++ )
                {
                    const void* pValue = pPropValue[ i ].Value.getValue();
                    if ( pValue )
                    {
                        ::rtl::OUString aPropName( pPropValue[ i ].Name );
                        if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "NumberingType" ) ) )
                            nNumberingType = *( (sal_Int16*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Adjust" ) ) )
                            nHorzAdjust = *( (sal_Int16*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletChar" ) ) )
                        {
                            String aString( *( (String*)pValue ) );
                            if ( aString.Len() )
                                cBulletId = aString.GetChar( 0 );
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletFont" ) ) )
                            aFontDesc = *( (::com::sun::star::awt::FontDescriptor*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "GraphicURL" ) ) )
                            aGraphicURL = ( *(::rtl::OUString*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "GraphicSize" ) ) )
                        {
                            if ( pPropValue[ i ].Value.getValueType() == ::getCppuType( (::com::sun::star::awt::Size*)0) )
                                aBuGraSize =  *(Size*)pValue;
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "StartWith" ) ) )
                            nStartWith = *( (sal_Int16*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "LeftMargin" ) ) )
                            nTextOfs = (sal_Int16)( *( (sal_Int32*)pValue ) / 4.40972 );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FirstLineOffset" ) ) )
                            nBulletOfs = (sal_Int16)( *( (sal_Int32*)pValue ) / 4.40972 );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletColor" ) ) )
                        {
                            sal_uInt32 nSOColor = *( (sal_uInt32*)pValue );
                            nBulletColor = nSOColor & 0xff00;                           // GRUEN
                            nBulletColor |= (sal_uInt8)( nSOColor ) << 16;              // ROT
                            nBulletColor |= (sal_uInt8)( nSOColor >> 16 ) | 0xfe000000; // BLAU
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletRelSize" ) ) )
                        {
                            nBulletRealSize = *( (sal_Int16*)pValue );
                            nParaFlags |= 0x40;
                            nBulletFlags |= 8;
                        }
#ifdef DBG_UTIL
                        else if ( ! (
                                ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SymbolTextDistance" ) ) )
                            ||  ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Prefix" ) ) )
                            ||  ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Suffix" ) ) )
                            ||  ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Graphic" ) ) ) ) )
                        {
                            DBG_ERROR( "Unbekanntes Property" );
                        }
#endif
                    }
                }

                if ( aGraphicURL.Len() )
                {
                    xub_StrLen nIndex = aGraphicURL.Search( (sal_Unicode)':', 0 );
                    if ( nIndex != STRING_NOTFOUND )
                    {
                        nIndex++;
                        if ( aGraphicURL.Len() > nIndex  )
                        {
                            ByteString aUniqueId( aGraphicURL, nIndex, aGraphicURL.Len() - nIndex, RTL_TEXTENCODING_UTF8 );
                            if ( aUniqueId.Len() )
                            {
                                nBulletId = rBuProv.GetId( aUniqueId, aBuGraSize );
                                if ( nBulletId != 0xffff )
                                    bExtendedBulletsUsed = TRUE;
                            }
                        }
                    }
                }

                PortionObj* pPortion = (PortionObj*)First();
                CalculateGraphicBulletSize( ( pPortion ) ? pPortion->mnCharHeight : 24 );

                switch( (SvxExtNumType)nNumberingType )
                {
                    case SVX_NUM_NUMBER_NONE : nParaFlags |= 0xf; break;

                    case SVX_NUM_CHAR_SPECIAL :                           // Bullet
                    {
                        if ( aFontDesc.Name.len() )
                        {
                            if ( aFontDesc.CharSet != ::com::sun::star::awt::CharSet::SYMBOL )
                            {
                                switch ( cBulletId )
                                {
                                    // Currency
                                    case 128:   cBulletId = 0x20AC; break;
                                    // Punctuation and other
                                    case 130:   cBulletId = 0x201A; break;// SINGLE LOW-9 QUOTATION MARK
                                    case 131:   cBulletId = 0x0192; break;// LATIN SMALL LETTER F WITH HOOK
                                    case 132:   cBulletId = 0x201E; break;// DOUBLE LOW-9 QUOTATION MARK
                                                                          // LOW DOUBLE PRIME QUOTATION MARK
                                    case 133:   cBulletId = 0x2026; break;// HORIZONTAL ELLIPSES
                                    case 134:   cBulletId = 0x2020; break;// DAGGER
                                    case 135:   cBulletId = 0x2021; break;// DOUBLE DAGGER
                                    case 136:   cBulletId = 0x02C6; break;// MODIFIER LETTER CIRCUMFLEX ACCENT
                                    case 137:   cBulletId = 0x2030; break;// PER MILLE SIGN
                                    case 138:   cBulletId = 0x0160; break;// LATIN CAPITAL LETTER S WITH CARON
                                    case 139:   cBulletId = 0x2039; break;// SINGLE LEFT-POINTING ANGLE QUOTATION MARK
                                    case 140:   cBulletId = 0x0152; break;// LATIN CAPITAL LIGATURE OE
                                    case 142:   cBulletId = 0x017D; break;// LATIN CAPITAL LETTER Z WITH CARON
                                    case 145:   cBulletId = 0x2018; break;// LEFT SINGLE QUOTATION MARK
                                                                          // MODIFIER LETTER TURNED COMMA
                                    case 146:   cBulletId = 0x2019; break;// RIGHT SINGLE QUOTATION MARK
                                                                          // MODIFIER LETTER APOSTROPHE
                                    case 147:   cBulletId = 0x201C; break;// LEFT DOUBLE QUOTATION MARK
                                                                          // REVERSED DOUBLE PRIME QUOTATION MARK
                                    case 148:   cBulletId = 0x201D; break;// RIGHT DOUBLE QUOTATION MARK
                                                                          // REVERSED DOUBLE PRIME QUOTATION MARK
                                    case 149:   cBulletId = 0x2022; break;// BULLET
                                    case 150:   cBulletId = 0x2013; break;// EN DASH
                                    case 151:   cBulletId = 0x2014; break;// EM DASH
                                    case 152:   cBulletId = 0x02DC; break;// SMALL TILDE
                                    case 153:   cBulletId = 0x2122; break;// TRADE MARK SIGN
                                    case 154:   cBulletId = 0x0161; break;// LATIN SMALL LETTER S WITH CARON
                                    case 155:   cBulletId = 0x203A; break;// SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
                                    case 156:   cBulletId = 0x0153; break;// LATIN SMALL LIGATURE OE
                                    case 158:   cBulletId = 0x017E; break;// LATIN SMALL LETTER Z WITH CARON
                                    case 159:   cBulletId = 0x0178; break;// LATIN CAPITAL LETTER Y WITH DIAERESIS
//                                  case 222:   cBulletId = 0x00B6; break;// PILCROW SIGN / PARAGRAPH SIGN
                                }
                            }
                            nParaFlags |= 0x90; // wir geben den Font und den Charset vor
                        }
                    }
                    case SVX_NUM_CHARS_UPPER_LETTER :       // zaehlt von a-z, aa - az, ba - bz, ...
                    case SVX_NUM_CHARS_LOWER_LETTER :
                    case SVX_NUM_ROMAN_UPPER :
                    case SVX_NUM_ROMAN_LOWER :
                    case SVX_NUM_ARABIC :
                    case SVX_NUM_PAGEDESC :                 // Numerierung aus der Seitenvorlage
                    case SVX_NUM_BITMAP :
                    case SVX_NUM_CHARS_UPPER_LETTER_N :     // zaehlt von  a-z, aa-zz, aaa-zzz
                    case SVX_NUM_CHARS_LOWER_LETTER_N :
                    {
                        if ( nNumberingType != SVX_NUM_CHAR_SPECIAL )
                        {
                            bExtendedBulletsUsed = TRUE;
                            if ( nDepth & 1 )
                                cBulletId = 0x2013;         // defaulting bullet characters for ppt97
                            else if ( nDepth == 4 )
                                cBulletId = 0xbb;
                            else
                                cBulletId = 0x2022;

                            switch( (SvxExtNumType)nNumberingType )
                            {
                                case SVX_NUM_CHARS_UPPER_LETTER :
                                case SVX_NUM_CHARS_UPPER_LETTER_N :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xa0001;   // (A)
                                        else
                                            nMappedNumType = 0xb0001;   // A)
                                    }
                                    else
                                        nMappedNumType = 0x10001;       // A.
                                }
                                break;
                                case SVX_NUM_CHARS_LOWER_LETTER :
                                case SVX_NUM_CHARS_LOWER_LETTER_N :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0x80001;   // (a)
                                        else
                                            nMappedNumType = 0x90001;   // a)
                                    }
                                    else
                                        nMappedNumType = 0x00001;       // a.
                                }
                                break;
                                case SVX_NUM_ROMAN_UPPER :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xe0001;   // (I)
                                        else
                                            nMappedNumType = 0xf0001;   // I)
                                    }
                                    else
                                        nMappedNumType = 0x70001;       // I.
                                }
                                break;
                                case SVX_NUM_ROMAN_LOWER :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0x40001;   // (i)
                                        else
                                            nMappedNumType = 0x50001;   // i)
                                    }
                                    else
                                        nMappedNumType = 0x60001;       // i.
                                }
                                break;
                                case SVX_NUM_ARABIC :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xc0001;   // (1)
                                        else
                                            nMappedNumType = 0x20001;   // 1)
                                    }
                                    else
                                    {
                                        if ( ! ( sSuffix.Len() + sPrefix.Len() ) )
                                            nMappedNumType = 0xd0001;   // 1
                                        else
                                            nMappedNumType = 0x30001;   // 1.
                                    }
                                }
                                break;
                            }
                        }
                        nParaFlags |= 0x2f;
                        nBulletFlags |= 6;
                        if ( mbIsBullet )
                            nBulletFlags |= 1;
                    }
                }
            }
            nBulletOfs = nTextOfs + nBulletOfs;
            if ( nBulletOfs < 0 )
                nBulletOfs = 0;
        }
    }
}

void ParagraphObj::ImplGetParagraphValues( PPTExBulletProvider& rBuProv, sal_Bool bGetPropStateValue )
{
    static String sIsNumbering      ( RTL_CONSTASCII_USTRINGPARAM( "IsNumbering" ) );
    static String sNumberingLevel   ( RTL_CONSTASCII_USTRINGPARAM( "NumberingLevel" ) );

    ::com::sun::star::uno::Any aAny;
    meBullet = ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    if ( GetPropertyValue( aAny, mXPropSet, sIsNumbering, sal_True ) )
    {
        if  ( bGetPropStateValue )
            meBullet = GetPropertyState( mXPropSet, sIsNumbering );
        aAny >>= mbIsBullet;

        if ( GetPropertyValue( aAny, mXPropSet, sNumberingLevel, sal_True ) )
        {
            if ( bGetPropStateValue )
                meBullet = GetPropertyState( mXPropSet, sNumberingLevel );
            nDepth = *( (sal_Int16*)aAny.getValue() );
            if ( nDepth > 4 )
                nDepth = 4;
            bDepth = TRUE;
        }
        else
            nDepth = 0;
        ImplGetNumberingLevel( rBuProv, nDepth, bGetPropStateValue );
    }
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaTabstops" ) ), bGetPropStateValue ) )
        maTabStop = *( ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop>*)mAny.getValue();
    ::com::sun::star::style::ParagraphAdjust eTextAdjust( ::com::sun::star::style::ParagraphAdjust_LEFT );
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaAdjust" ) ), bGetPropStateValue ) )
        eTextAdjust = (::com::sun::star::style::ParagraphAdjust)EncodeAnyTosal_Int16( mAny );
    switch ( eTextAdjust )
    {
        case ::com::sun::star::style::ParagraphAdjust_CENTER :
            mnTextAdjust = 1;
        break;
        case ::com::sun::star::style::ParagraphAdjust_RIGHT :
            mnTextAdjust = 2;
        break;
        case ::com::sun::star::style::ParagraphAdjust_BLOCK :
            mnTextAdjust = 3;
        break;
        default :
        case ::com::sun::star::style::ParagraphAdjust_LEFT :
            mnTextAdjust = 0;
        break;
    }
    meTextAdjust = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaLineSpacing" ) ), bGetPropStateValue ) )
    {
        ::com::sun::star::style::LineSpacing aLineSpacing
            = *( (::com::sun::star::style::LineSpacing*)mAny.getValue() );
        switch ( aLineSpacing.Mode )
        {
            case ::com::sun::star::style::LineSpacingMode::MINIMUM :
            case ::com::sun::star::style::LineSpacingMode::LEADING :
            case ::com::sun::star::style::LineSpacingMode::FIX :
                mnLineSpacing = (sal_Int16)(-( aLineSpacing.Height / 4.40972 ) );
            break;

            case ::com::sun::star::style::LineSpacingMode::PROP :
            default:
                mnLineSpacing = (sal_Int16)( aLineSpacing.Height );
            break;
        }
    }
    meLineSpacing = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaBottomMargin" ) ), bGetPropStateValue ) )
    {
        sal_uInt32 nSpacing = *( (sal_uInt32*)mAny.getValue() );
        mnLineSpacingBottom = (sal_Int16)(-( nSpacing / 4.40972 ) );
    }
    meLineSpacingBottom = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaTopMargin" ) ), bGetPropStateValue ) )
    {
        sal_uInt32 nSpacing = *( (sal_uInt32*)mAny.getValue() );
        mnLineSpacingTop = (sal_Int16)(-( nSpacing / 4.40972 ) );
    }
    meLineSpacingTop = ePropState;
}

void ParagraphObj::ImplConstruct( ParagraphObj& rParagraphObj )
{
    mnTextSize = rParagraphObj.mnTextSize;
    mnTextAdjust = rParagraphObj.mnTextAdjust;
    mnLineSpacing = rParagraphObj.mnLineSpacing;
    mnLineSpacingTop = rParagraphObj.mnLineSpacingTop;
    mnLineSpacingBottom = rParagraphObj.mnLineSpacingBottom;
    mbFirstParagraph = rParagraphObj.mbFirstParagraph;
    mbLastParagraph = rParagraphObj.mbLastParagraph;

    for ( void* pPtr = rParagraphObj.First(); pPtr; pPtr = rParagraphObj.Next() )
        Insert( new PortionObj( *(PortionObj*)pPtr ), LIST_APPEND );

    maTabStop = rParagraphObj.maTabStop;
    bDepth = rParagraphObj.bDepth;
    bExtendedParameters = rParagraphObj.bExtendedParameters;
    nParaFlags = rParagraphObj.nParaFlags;
    nBulletFlags = rParagraphObj.nBulletFlags;
    sPrefix = rParagraphObj.sPrefix;
    sSuffix = rParagraphObj.sSuffix;
    sGraphicUrl = rParagraphObj.sGraphicUrl;            // String auf eine Graphic
    aBuGraSize = rParagraphObj.aBuGraSize;
    nNumberingType = rParagraphObj.nNumberingType;      // in wirlichkeit ist dies ein SvxEnum
    nHorzAdjust = rParagraphObj.nHorzAdjust;
    nBulletColor = rParagraphObj.nBulletColor;
    nBulletOfs = rParagraphObj.nBulletOfs;
    nStartWith = rParagraphObj.nStartWith;              // Start der nummerierung
    nTextOfs = rParagraphObj.nTextOfs;
    nBulletRealSize = rParagraphObj.nBulletRealSize;    // GroessenVerhaeltnis in Proz
    nDepth = rParagraphObj.nDepth;                      // aktuelle tiefe
    cBulletId = rParagraphObj.cBulletId;                // wenn Numbering Type == CharSpecial
    aFontDesc = rParagraphObj.aFontDesc;

    bExtendedBulletsUsed = rParagraphObj.bExtendedBulletsUsed;
    nBulletId = rParagraphObj.nBulletId;
}

::com::sun::star::awt::Size ParagraphObj::ImplMapSize( const ::com::sun::star::awt::Size& rSize )
{
    Size aSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );
    if ( !aSize.Width() )
        aSize.Width()++;
    if ( !aSize.Height() )
        aSize.Height()++;
    return ::com::sun::star::awt::Size( aSize.Width(), aSize.Height() );
}

sal_uInt32 ParagraphObj::ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition )
{
    mnTextSize = 0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        mnTextSize += ((PortionObj*)pPtr)->ImplCalculateTextPositions( nCurrentTextPosition + mnTextSize );
    return mnTextSize;
}

ParagraphObj& ParagraphObj::operator=( ParagraphObj& rParagraphObj )
{
    if ( this != &rParagraphObj )
    {
        ImplClear();
        ImplConstruct( rParagraphObj );
    }
    return *this;
}

//  -----------------------------------------------------------------------

ImplTextObj::ImplTextObj( int nInstance )
{
    mnRefCount = 1;
    mnTextSize = 0;
    mnInstance = nInstance;
    mpList = new List;
    mbHasExtendedBullets = FALSE;
}

ImplTextObj::~ImplTextObj()
{
    for ( ParagraphObj* pPtr = (ParagraphObj*)mpList->First(); pPtr; pPtr = (ParagraphObj*)mpList->Next() )
        delete pPtr;
    delete mpList;
}

TextObj::TextObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XSimpleText > & rXTextRef,
            int nInstance, FontCollection& rFontCollection, PPTExBulletProvider& rProv )
{
    mpImplTextObj = new ImplTextObj( nInstance );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
        aXTextParagraphEA( rXTextRef, ::com::sun::star::uno::UNO_QUERY );

    if ( aXTextParagraphEA.is()  )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
            aXTextParagraphE( aXTextParagraphEA->createEnumeration() );
        if ( aXTextParagraphE.is() )
        {
            ParaFlags aParaFlags;
            while ( aXTextParagraphE->hasMoreElements() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > aXParagraph;
                ::com::sun::star::uno::Any aAny( aXTextParagraphE->nextElement() );
                if ( aAny >>= aXParagraph )
                {
                    if ( !aXTextParagraphE->hasMoreElements() )
                        aParaFlags.bLastParagraph = TRUE;
                    ParagraphObj* pPara = new ParagraphObj( aXParagraph, aParaFlags, rFontCollection, rProv );
                    mpImplTextObj->mbHasExtendedBullets |= pPara->bExtendedBulletsUsed;
                    mpImplTextObj->mpList->Insert( pPara, LIST_APPEND );
                    aParaFlags.bFirstParagraph = FALSE;
                }
            }
        }
    }
    ImplCalculateTextPositions();
}

TextObj::TextObj( TextObj& rTextObj )
{
    mpImplTextObj = rTextObj.mpImplTextObj;
    mpImplTextObj->mnRefCount++;
}

TextObj::~TextObj()
{
    if ( ! ( --mpImplTextObj->mnRefCount ) )
        delete mpImplTextObj;
}

void TextObj::Write( SvStream* pStrm )
{
    sal_uInt32 nSize, nPos = pStrm->Tell();
    *pStrm << (sal_uInt32)( EPP_TextCharsAtom << 16 ) << (sal_uInt32)0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        ((ParagraphObj*)pPtr)->Write( pStrm );
    nSize = pStrm->Tell() - nPos;
    pStrm->SeekRel( - ( (sal_Int32)nSize - 4 ) );
    *pStrm << (sal_uInt32)( nSize - 8 );
    pStrm->SeekRel( nSize - 8 );
}

void TextObj::ImplCalculateTextPositions()
{
    mpImplTextObj->mnTextSize = 0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        mpImplTextObj->mnTextSize += ((ParagraphObj*)pPtr)->ImplCalculateTextPositions( mpImplTextObj->mnTextSize );
}

TextObj& TextObj::operator=( TextObj& rTextObj )
{
    if ( this != &rTextObj )
    {
        if ( ! ( --mpImplTextObj->mnRefCount ) )
            delete mpImplTextObj;
        mpImplTextObj = rTextObj.mpImplTextObj;
        mpImplTextObj->mnRefCount++;
    }
    return *this;
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteTextStyleAtom( SvStream& rOut, int nTextInstance,
                    sal_uInt32 nAtomInstance, TextRuleEntry* pTextRule, SvStream& rExtBuStr )
{
    PPTExParaSheet& rParaSheet = mpStyleSheet->GetParaSheet( nTextInstance );

    rOut << (sal_uInt32)( ( EPP_TextHeaderAtom << 16 ) | ( nAtomInstance << 4 ) ) << (sal_uInt32)4
         << nTextInstance;

    List* pFieldList = NULL;

    if ( mbEmptyPresObj )
        mnTextSize = 0;
    if ( mnTextSize )
    {
        TextObj aTextObj( mXText, nTextInstance, maFontCollection, (PPTExBulletProvider&)*this );
        aTextObj.Write( &rOut );

        sal_uInt32 nSize, nPos = rOut.Tell();
        rOut << (sal_uInt32)( EPP_StyleTextPropAtom << 16 ) << (sal_uInt32)0;
        ImplWriteParagraphs( rOut, aTextObj );
        ImplWritePortions( rOut, aTextObj );
        nSize = rOut.Tell() - nPos;
        rOut.SeekRel( - ( (sal_Int32)nSize - 4 ) );
        rOut << (sal_uInt32)( nSize - 8 );
        rOut.SeekRel( nSize - 8 );

        for ( ParagraphObj* pPara = aTextObj.First(); pPara; pPara = aTextObj.Next() )
        {
            for ( PortionObj* pPortion = (PortionObj*)pPara->First(); pPortion; pPortion = (PortionObj*)pPara->Next() )
            {
                if ( pPortion->mpFieldEntry )
                {
                    const FieldEntry* pFieldEntry = pPortion->mpFieldEntry;

                    switch ( pFieldEntry->nFieldType >> 28 )
                    {
                        case 1 :
                        case 2 :
                        {
                            rOut << (sal_uInt32)( EPP_DateTimeMCAtom << 16 ) << (sal_uInt32)8
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos )          // TxtOffset auf TxtField;
                                 << (sal_uInt8)( ( pFieldEntry->nFieldType >> 24 ) & 0xf )// Type
                                 << (sal_uInt8)0 << (sal_uInt16)0;                            // PadBytes
                        }
                        break;
                        case 3 :
                        {
                            rOut << (sal_uInt32)( EPP_SlideNumberMCAtom << 16 ) << (sal_uInt32 ) 4
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos );
                        }
                        break;
                        case 4 :
                        {
                            sal_uInt32 nHyperId = ++mnExEmbed;

                            rOut << (sal_uInt32)( ( EPP_InteractiveInfo << 16 ) | 0xf ) << (sal_uInt32)24
                                 << (sal_uInt32)( EPP_InteractiveInfoAtom << 16 ) << (sal_uInt32)16
                                 << (sal_uInt32)0                                    // soundref
                                 << nHyperId                                     // hyperlink id
                                 << (sal_uInt8)4                                      // hyperlink action
                                 << (sal_uInt8)0                                      // ole verb
                                 << (sal_uInt8)0                                      // jump
                                 << (sal_uInt8)0                                      // flags
                                 << (sal_uInt8)8                                      // hyperlink type ?
                                 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0
                                 << (sal_uInt32)( EPP_TxInteractiveInfoAtom << 16 ) << (sal_uInt32)8
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos )
                                 << (sal_uInt32)( pFieldEntry->nFieldEndPos );

                            maHyperlink.Insert( new EPPTHyperlink( pFieldEntry->aFieldUrl, 2 | ( nHyperId << 8 ) ), LIST_APPEND );

                            *mpExEmbed  << (sal_uInt16)0xf
                                        << (sal_uInt16)EPP_ExHyperlink
                                        << (sal_uInt32)12
                                        << (sal_uInt16)0
                                        << (sal_uInt16)EPP_ExHyperlinkAtom
                                        << (sal_uInt32)4
                                        << nHyperId;
                        }
                        default:
                        break;
                    }
                }
            }
        }
        // SJ: if this textspecinfoatom is missing, ppt will crash as often as possible
        // MS documentation : special parsing code is needed to parse this content !?!
        rOut << (sal_uInt32)( EPP_TextSpecInfoAtom << 16 ) << (sal_uInt32)10            // ??????????????????????????
             << (sal_uInt32)aTextObj.Count() << (sal_uInt32)2 << (sal_uInt8)9 << (sal_uInt8)8;  // ??????????????????????????

        // Star Office Default TabSizes schreiben ( wenn noetig )
        pPara = aTextObj.First();
        if ( pPara )
        {
            sal_uInt32  nParaFlags = 0x1f;
            sal_Int16   nDepth, nMask, nNumberingRule[ 10 ];
            sal_uInt32  nTextOfs = pPara->nTextOfs;
            sal_uInt32  nTabs = pPara->maTabStop.getLength();
            const ::com::sun::star::style::TabStop* pTabStop = ( const ::com::sun::star::style::TabStop* )pPara->maTabStop.getConstArray();

            for ( ; pPara; pPara = aTextObj.Next() )
            {
                if ( pPara->bExtendedParameters )
                {
                    nDepth = pPara->nDepth;
                    if ( nDepth < 5 )
                    {
                        nMask = 1 << nDepth;
                        if ( nParaFlags & nMask )
                        {
                            nParaFlags &=~ nMask;
                            if ( ( rParaSheet.maParaLevel[ nDepth ].mnTextOfs != pPara->nTextOfs ) ||
                                ( rParaSheet.maParaLevel[ nDepth ].mnBulletOfs != pPara->nBulletOfs ) )
                            {
                                nParaFlags |= nMask << 16;
                                nNumberingRule[ nDepth << 1 ] = pPara->nTextOfs;
                                nNumberingRule[ ( nDepth << 1 ) + 1 ] = (sal_Int16)pPara->nBulletOfs;
                            }
                        }
                    }
                }
            }
            nParaFlags >>= 16;

            sal_uInt32  nDefaultTabSize = ImplMapSize( ::com::sun::star::awt::Size( 2011, 1 ) ).Width;
            sal_uInt32  nDefaultTabs = abs( maRect.GetWidth() ) / nDefaultTabSize;
            if ( nTabs )
                nDefaultTabs -= (sal_Int32)( ( ( pTabStop[ nTabs - 1 ].Position / 4.40972 ) + nTextOfs ) / nDefaultTabSize );
            if ( (sal_Int32)nDefaultTabs < 0 )
                nDefaultTabs = 0;

            sal_uInt32 nTabCount = nTabs + nDefaultTabs;
            sal_uInt32 i, nTextRulerAtomFlags = 0;

            if ( nTabCount )
                nTextRulerAtomFlags |= 4;
            if ( nParaFlags )
                nTextRulerAtomFlags |= ( ( nParaFlags << 3 ) | ( nParaFlags << 8 ) );

            if ( nTextRulerAtomFlags )
            {
                SvStream* pRuleOut = &rOut;
                if ( pTextRule )
                    pRuleOut = pTextRule->pOut = new SvMemoryStream( 0x100, 0x100 );

                sal_uInt32 nPos = pRuleOut->Tell();
                *pRuleOut << (sal_uInt32)( EPP_TextRulerAtom << 16 ) << (sal_uInt32)0;
                *pRuleOut << nTextRulerAtomFlags;
                if ( nTextRulerAtomFlags & 4 )
                {
                    *pRuleOut << (sal_uInt16)nTabCount;
                    for ( i = 0; i < nTabs; i++ )
                    {
                        sal_uInt16 nPosition = (sal_uInt16)( ( pTabStop[ i ].Position / 4.40972 ) + nTextOfs );
                        sal_uInt16 nType;
                        switch ( pTabStop[ i ].Alignment )
                        {
                            case ::com::sun::star::style::TabAlign_DECIMAL :    nType = 3; break;
                            case ::com::sun::star::style::TabAlign_RIGHT :      nType = 2; break;
                            case ::com::sun::star::style::TabAlign_CENTER :     nType = 1; break;

                            case ::com::sun::star::style::TabAlign_LEFT :
                            default:                                            nType = 0;
                        };
                        *pRuleOut << nPosition
                                  << nType;
                    }

                    sal_uInt32 nWidth = 1;
                    if ( nTabs )
                        nWidth += (sal_Int32)( ( ( pTabStop[ nTabs - 1 ].Position / 4.40972 + nTextOfs ) / nDefaultTabSize ) );
                    nWidth *= nDefaultTabSize;
                    for ( i = 0; i < nDefaultTabs; i++, nWidth += nDefaultTabSize )
                        *pRuleOut << nWidth;
                }
                for ( i = 0; i < 5; i++ )
                {
                    if ( nTextRulerAtomFlags & ( 8 << i ) )
                        *pRuleOut << nNumberingRule[ i << 1 ];
                    if ( nTextRulerAtomFlags & ( 256 << i ) )
                        *pRuleOut << nNumberingRule[ ( i << 1 ) + 1 ];
                }
                sal_uInt32 nBufSize = pRuleOut->Tell() - nPos;
                pRuleOut->SeekRel( - ( (sal_Int32)nBufSize - 4 ) );
                *pRuleOut << (sal_uInt32)( nBufSize - 8 );
                pRuleOut->SeekRel( nBufSize - 8 );
            }
        }
        if ( aTextObj.HasExtendedBullets() )
        {
            ParagraphObj* pPara = aTextObj.First();
            if ( pPara )
            {
                sal_uInt32  nBulletFlags;
                sal_uInt32  nNumberingType, nPos = rExtBuStr.Tell();

                rExtBuStr << (sal_uInt32)( EPP_PST_ExtendedParagraphAtom << 16 ) << (sal_uInt32)0;

                for ( ; pPara; pPara = aTextObj.Next() )
                {
                    nBulletFlags = 0;
                    sal_uInt16 nBulletId = pPara->nBulletId;
                    if ( pPara->bExtendedBulletsUsed )
                    {
                        nBulletFlags = 0x800000;
                        if ( pPara->nNumberingType != SVX_NUM_BITMAP )
                            nBulletFlags = 0x3000000;
                    }
                    rExtBuStr << (sal_uInt32)nBulletFlags;

                    if ( nBulletFlags & 0x800000 )
                        rExtBuStr << nBulletId;
                    if ( nBulletFlags & 0x1000000 )
                    {
                        switch( pPara->nNumberingType )
                        {
                            case SVX_NUM_NUMBER_NONE :
                            case SVX_NUM_CHAR_SPECIAL :
                                nNumberingType = 0;
                            break;
                            case SVX_NUM_CHARS_UPPER_LETTER :
                            case SVX_NUM_CHARS_UPPER_LETTER_N :
                            case SVX_NUM_CHARS_LOWER_LETTER :
                            case SVX_NUM_CHARS_LOWER_LETTER_N :
                            case SVX_NUM_ROMAN_UPPER :
                            case SVX_NUM_ROMAN_LOWER :
                            case SVX_NUM_ARABIC :
                                nNumberingType = pPara->nMappedNumType;
                            break;

    //                      case SVX_NUM_PAGEDESC :
                            case SVX_NUM_BITMAP :
                                nNumberingType = 0;
                            break;

                        }
                        rExtBuStr << (sal_uInt32)nNumberingType;
                    }
                    if ( nBulletFlags & 0x2000000 )
                        rExtBuStr << (sal_uInt16)pPara->nStartWith;
                    rExtBuStr << (sal_uInt32)0 << (sal_uInt32)0;
                }
                sal_uInt32 nSize = ( rExtBuStr.Tell() - nPos ) - 8;
                rExtBuStr.SeekRel( - ( (sal_Int32)nSize + 4 ) );
                rExtBuStr << nSize;
                rExtBuStr.SeekRel( nSize );
            }
        }
    }
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteObjectEffect( SvStream& rSt,
    ::com::sun::star::presentation::AnimationEffect eAe,
    ::com::sun::star::presentation::AnimationEffect eTe,
    sal_uInt16 nOrder )
{
    sal_uInt32  nDimColor = 0x7000000;  // color to use for dimming
    sal_uInt32  nFlags = 0x4400;        // set of flags that determine type of build
    sal_uInt32  nSoundRef = 0;          // 0 if storage is from clipboard. Otherwise index(ID) in SoundCollection list.
    sal_uInt32  nDelayTime = 0;         // delay before playing object
    sal_uInt16  nSlideCount = 1;        // number of slides to play object
    UINT8   nBuildType = 1;         // type of build
    UINT8   nFlyMethod = 0;         // animation effect( fly, zoom, appear, etc )
    UINT8   nFlyDirection = 0;      // Animation direction( left, right, up, down, etc )
    UINT8   nAfterEffect = 0;       // what to do after build
    UINT8   nSubEffect = 0;         // build by word or letter
    UINT8   nOleVerb = 0;           // Determines object's class (sound, video, other)

    if ( eAe == ::com::sun::star::presentation::AnimationEffect_NONE )
    {
        nBuildType = 0;
        eAe = eTe;
    }
    switch ( eAe )
    {
        case ::com::sun::star::presentation::AnimationEffect_NONE :
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LEFT :
        {
            nFlyDirection = 2;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_TOP :
        {
            nFlyDirection = 3;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_RIGHT :
        {
            nFlyDirection = 0;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_BOTTOM :
        {
            nFlyDirection = 1;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER :
        {
            nFlyDirection = 1;
            nFlyMethod = 11;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER :
        {
            nFlyDirection = 0;
            nFlyMethod = 11;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LEFT :
        {
            nFlyDirection = 0;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_TOP :
        {
            nFlyDirection = 1;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_RIGHT :
        {
            nFlyDirection = 2;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_BOTTOM :
        {
            nFlyDirection = 3;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_VERTICAL_STRIPES :
        {
            nFlyDirection = 0;
            nFlyMethod = 2;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_STRIPES :
        {
            nFlyDirection = 1;
            nFlyMethod = 2;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_CLOCKWISE :
        {
            nFlyDirection = 1;
            nFlyMethod = 3;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_COUNTERCLOCKWISE :
        {
            nFlyDirection = 0;
            nFlyMethod = 3;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERLEFT :
        {
            nFlyDirection = 7;
            nFlyMethod = 9;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERRIGHT :
        {
            nFlyDirection = 6;
            nFlyMethod = 9;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERLEFT :
        {
            nFlyDirection = 5;
            nFlyMethod = 9;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERRIGHT :
        {
            nFlyDirection = 4;
            nFlyMethod = 9;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_CLOSE_VERTICAL :
        {
            nFlyDirection = 1;
            nFlyMethod = 13;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_CLOSE_HORIZONTAL :
        {
            nFlyDirection = 3;
            nFlyMethod = 13;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_OPEN_VERTICAL :
        {
            nFlyDirection = 0;
            nFlyMethod = 13;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_OPEN_HORIZONTAL :
        {
            nFlyDirection = 2;
            nFlyMethod = 13;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_PATH :
        {
            nFlyDirection = 28;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_LEFT :
        {
            nFlyDirection = 0;
            nFlyMethod = 1;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_TOP :
        {
            nFlyDirection = 0;
            nFlyMethod = 1;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_RIGHT :
        {
            nFlyDirection = 0;
            nFlyMethod = 1;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_BOTTOM :
        {
            nFlyDirection = 0;
            nFlyMethod = 1;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_SPIRALIN_LEFT :
        {
            nFlyDirection = 1;
            nFlyMethod = 3;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_SPIRALIN_RIGHT :
        {
            nFlyDirection = 0;
            nFlyMethod = 3;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_SPIRALOUT_LEFT :
        {
            nFlyDirection = 0;
            nFlyMethod = 3;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_SPIRALOUT_RIGHT :
        {
            nFlyDirection = 0;
            nFlyMethod = 3;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_DISSOLVE :
        {
            nFlyDirection = 0;
            nFlyMethod = 5;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_LEFT :
        {
            nFlyDirection = 2;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_TOP :
        {
            nFlyDirection = 3;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_RIGHT :
        {
            nFlyDirection = 0;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_BOTTOM :
        {
            nFlyDirection = 1;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_RANDOM :
        {
            nFlyDirection = 0;
            nFlyMethod = 1;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_VERTICAL_LINES :
        {
            nFlyDirection = 1;
            nFlyMethod = 8;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_LINES :
        {
            nFlyDirection = 0;
            nFlyMethod = 8;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_LEFT :
        {
            nFlyDirection = 2;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_TOP :
        {
            nFlyDirection = 3;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_RIGHT :
        {
            nFlyDirection = 0;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_BOTTOM :
        {
            nFlyDirection = 1;
            nFlyMethod = 10;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_UPPERLEFT :
        {
            nFlyDirection = 7;
            nFlyMethod = 9;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_UPPERRIGHT :
        {
            nFlyDirection = 6;
            nFlyMethod = 9;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_LOWERLEFT :
        {
            nFlyDirection = 5;
            nFlyMethod = 9;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_LOWERRIGHT :
        {
            nFlyDirection = 4;
            nFlyMethod = 9;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_APPEAR :
        break;
        case ::com::sun::star::presentation::AnimationEffect_HIDE :
        {
            nFlyDirection = 0;
            nFlyMethod = 1;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_UPPERLEFT :
        {
            nFlyDirection = 4;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_UPPERRIGHT :
        {
            nFlyDirection = 5;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LOWERRIGHT :
        {
            nFlyDirection = 7;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LOWERLEFT :
        {
            nFlyDirection = 6;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_UPPERLEFT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_UPPERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_LOWERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_LOWERLEFT :
            nAfterEffect |= 2;
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_LEFT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_UPPERLEFT :
        {
            nFlyDirection = 8;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_TOP :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_UPPERRIGHT :
        {
            nFlyDirection = 11;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_RIGHT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_LOWERRIGHT :
        {
            nFlyDirection = 10;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_BOTTOM :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_LOWERLEFT :
        {
            nFlyDirection = 9;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_LEFT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_UPPERLEFT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_TOP :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_RIGHT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_BOTTOM :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_LOWERLEFT :
            nAfterEffect |= 2;
        break;
        case ::com::sun::star::presentation::AnimationEffect_VERTICAL_CHECKERBOARD :
        {
            nFlyDirection = 1;
            nFlyMethod = 3;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_CHECKERBOARD :
        {
            nFlyDirection = 0;
            nFlyMethod = 3;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_ROTATE :
        case ::com::sun::star::presentation::AnimationEffect_VERTICAL_ROTATE :
        {
            nFlyDirection = 27;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_STRETCH :
        case ::com::sun::star::presentation::AnimationEffect_VERTICAL_STRETCH :
        {
            nFlyDirection = 22;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_LEFT :
        case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_UPPERLEFT :
        {
            nFlyDirection = 23;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_TOP :
        case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_UPPERRIGHT :
        {
            nFlyDirection = 24;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_RIGHT :
        case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_LOWERRIGHT :
        {
            nFlyDirection = 25;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_BOTTOM :
        case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_LOWERLEFT :
        {
            nFlyDirection = 26;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN :
        {
            nFlyDirection = 16;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_SMALL :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_SPIRAL :
        {
            nFlyDirection = 17;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT :
        {
            nFlyDirection = 18;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_SMALL :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_SPIRAL :
        {
            nFlyDirection = 19;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_LEFT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_UPPERLEFT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_TOP :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_UPPERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_RIGHT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_LOWERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_BOTTOM :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_LOWERLEFT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_CENTER :
        {
            nFlyDirection = 16;
            nFlyMethod = 12;
        }
        break;
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_LEFT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_UPPERLEFT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_TOP :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_UPPERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_RIGHT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_LOWERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_BOTTOM :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_LOWERLEFT :
        case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_CENTER :
            nAfterEffect |= 2;
        break;
    }
    if ( mnDiaMode >= 1 )
        nFlags |= 4;
    if ( eTe != ::com::sun::star::presentation::AnimationEffect_NONE )
        nBuildType = 2;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "SoundOn" ) ) ) )
    {
        sal_Bool bBool;
        mAny >>= bBool;
        if ( bBool )
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Sound" ) ) ) )
            {
                nSoundRef = maSoundCollection.GetId( *(::rtl::OUString*)mAny.getValue() );
                if ( nSoundRef )
                    nFlags |= 0x10;
            }
        }
    }
    sal_Bool bDimHide = FALSE;
    sal_Bool bDimPrevious = FALSE;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DimHide" ) ) ) )
        mAny >>= bDimHide;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DimPrevious" ) ) ) )
        mAny >>= bDimPrevious;
    if ( bDimPrevious )
        nAfterEffect |= 1;
    if ( bDimHide )
        nAfterEffect |= 2;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DimColor" ) ) ) )
        nDimColor = mpPptEscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) ) | 0xfe000000;

    rSt << (sal_uInt32)( ( EPP_AnimationInfo << 16 ) | 0xf )  << (sal_uInt32)36
        << (sal_uInt32)( ( EPP_AnimationInfoAtom << 16 ) | 1 )<< (sal_uInt32)28
        << nDimColor << nFlags << nSoundRef << nDelayTime
        << nOrder                                   // order of build ( 1.. )
        << nSlideCount << nBuildType << nFlyMethod << nFlyDirection
        << nAfterEffect << nSubEffect << nOleVerb
        << (sal_uInt16)0;                               // PadWord
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteClickAction( SvStream& rSt, ::com::sun::star::presentation::ClickAction eCa )
{
    sal_uInt32 nSoundRef = 0;   // a reference to a sound in the sound collection, or NULL.
    sal_uInt32 nHyperLinkID = 0;// a persistent unique identifier to an external hyperlink object (only valid when action == HyperlinkAction).
    sal_uInt8   nAction = 0;     // Action See Action Table
    sal_uInt8   nOleVerb = 0;    // OleVerb Only valid when action == OLEAction. OLE verb to use, 0 = first verb, 1 = second verb, etc.
    sal_uInt8   nJump = 0;       // Jump See Jump Table
    sal_uInt8   nFlags = 0;      // Bit 1: Animated. If 1, then button is animated
                            // Bit 2: Stop sound. If 1, then stop current sound when button is pressed.
                            // Bit 3: CustomShowReturn. If 1, and this is a jump to custom show, then return to this slide after custom show.
    sal_uInt8   nHyperLinkType = 0;// HyperlinkType a value from the LinkTo enum, such as LT_URL (only valid when action == HyperlinkAction).

    String  aFile;

    /*
        Action Table:       Action Value
        NoAction            0
        MacroAction         1
        RunProgramAction    2
        JumpAction          3
        HyperlinkAction     4
        OLEAction           5
        MediaAction         6
        CustomShowAction    7

        Jump Table:     Jump Value
        NoJump          0
        NextSlide,      1
        PreviousSlide,  2
        FirstSlide,     3
        LastSlide,      4
        LastSlideViewed 5
        EndShow         6
    */

    switch( eCa )
    {
        case ::com::sun::star::presentation::ClickAction_STOPPRESENTATION :
            nJump += 2;
        case ::com::sun::star::presentation::ClickAction_LASTPAGE :
            nJump++;
        case ::com::sun::star::presentation::ClickAction_FIRSTPAGE :
            nJump++;
        case ::com::sun::star::presentation::ClickAction_PREVPAGE :
            nJump++;
        case ::com::sun::star::presentation::ClickAction_NEXTPAGE :
        {
            nJump++;
            nAction = 3;
        }
        break;
        case ::com::sun::star::presentation::ClickAction_SOUND :
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
                nSoundRef = maSoundCollection.GetId( *(::rtl::OUString*)mAny.getValue() );
        }
        break;
        case ::com::sun::star::presentation::ClickAction_PROGRAM :
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
            {
                INetURLObject aUrl( *(::rtl::OUString*)mAny.getValue() );
                if ( INET_PROT_FILE == aUrl.GetProtocol() )
                {
                    aFile = aUrl.PathToFileName();
                    nAction = 2;
                }
            }
        }
        break;

        case ::com::sun::star::presentation::ClickAction_BOOKMARK :
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
            {
                String  aBookmark( *(::rtl::OUString*)mAny.getValue() );
                sal_uInt32 nIndex = 0;
                for ( String* pStr = (String*)maSlideNameList.First(); pStr; pStr = (String*)maSlideNameList.Next(), nIndex++ )
                {
                    if ( *pStr == aBookmark )
                    {
                        // Bookmark ist ein link zu einer Dokumentseite
                        nAction = 4;
                        nHyperLinkID = ++mnExEmbed;                 //( maHyperlink.Count() + 1 ) << 1;
                        nHyperLinkType = 7;
                        String  aHyperString = UniString::CreateFromInt32( 256 + nIndex );
                        aHyperString.Append( String( RTL_CONSTASCII_USTRINGPARAM( "," ) ) );
                        aHyperString.Append( String::CreateFromInt32( nIndex + 1 ) );
                        aHyperString.Append( String( RTL_CONSTASCII_USTRINGPARAM( ",Slide " ) ) );
                        aHyperString.Append( String::CreateFromInt32( nIndex + 1 ) );
                        maHyperlink.Insert( new EPPTHyperlink( aHyperString, 1 | ( nIndex << 8 ) | ( 1 << 31 ) ), LIST_APPEND );

                        *mpExEmbed  << (sal_uInt16)0xf
                                    << (sal_uInt16)EPP_ExHyperlink
                                    << (sal_uInt32)12
                                    << (sal_uInt16)0
                                    << (sal_uInt16)EPP_ExHyperlinkAtom
                                    << (sal_uInt32)4
                                    << nHyperLinkID;
                        break;
                    }
                }
            }
        }
        break;

        case ::com::sun::star::presentation::ClickAction_DOCUMENT :
        case ::com::sun::star::presentation::ClickAction_INVISIBLE :
        case ::com::sun::star::presentation::ClickAction_VERB :
        case ::com::sun::star::presentation::ClickAction_VANISH :
        case ::com::sun::star::presentation::ClickAction_MACRO :
        default :
        break;
    }

    sal_uInt32 nContainerSize = 24;
    if ( nAction == 2 )
        nContainerSize += ( aFile.Len() * 2 ) + 8;
    rSt << (sal_uInt32)( ( EPP_InteractiveInfo << 16 ) | 0xf ) << (sal_uInt32)nContainerSize
        << (sal_uInt32)( EPP_InteractiveInfoAtom << 16 ) << (sal_uInt32)16
        << nSoundRef
        << nHyperLinkID
        << nAction
        << nOleVerb
        << nJump
        << nFlags
        << (sal_uInt32)nHyperLinkType;

    if ( nAction == 2 )     // run program Action
    {
        sal_uInt16 i, nLen = aFile.Len();
        rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 0x20 ) << (sal_uInt32)( nLen * 2 );
        for ( i = 0; i < nLen; i++ )
            rSt << aFile.GetChar( i );
    }

    rSt << (sal_uInt32)( ( EPP_InteractiveInfo << 16 ) | 0x1f ) << (sal_uInt32)24   // Mouse Over Action
        << (sal_uInt32)( EPP_InteractiveInfo << 16 ) << (sal_uInt32)16;
    for ( int i = 0; i < 4; i++, rSt << (sal_uInt32)0 );
}

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplIsAutoShape ( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropertySet,
                                            sal_Bool bIsGroup, sal_Int32 nAngle, sal_uInt32& nShapeType, sal_uInt32& nReplace, List& rAdjustmentList,
                                                Rectangle& rPolyBoundRect )
{
    sal_Bool    bIsAutoShape = FALSE;
    try
    {
        sal_uInt32  i;
        sal_uInt32  nSequenceCount = 0;
        sal_uInt32* pPtr = NULL;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > aXIndexAccess;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > aXShape( rXShape );
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXPropSet( rXPropertySet );
        ::com::sun::star::uno::Sequence< sal_uInt32 > aSequence;

        if ( !bIsGroup )
            nReplace = 1;
        else
        {
            aXIndexAccess = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                            ( rXShape, ::com::sun::star::uno::UNO_QUERY );

            if ( !aXIndexAccess.is() )
                return FALSE;
            nReplace = aXIndexAccess->getCount();
            if ( !nReplace )
                return FALSE;
        }

        sal_uInt32 nChecksum = 0;
        sal_Int32 aVal[ 3 ];
        double fAngle;
        if ( nAngle )
            fAngle = F_PI18000 * ( 36000 - ( nAngle % 36000 ) );

        ::com::sun::star::awt::Point aPos( rXShape->getPosition() );
        ::com::sun::star::awt::Size  aSize( rXShape->getSize() );
        ::com::sun::star::awt::Point aCenter( aPos.X + ( aSize.Width >> 1 ), aPos.Y + ( aSize.Height >> 1 ) );

        for ( i = 0; i < nReplace; i++ )
        {
            sal_Bool    bBezier;
            ByteString  aType;

            if ( bIsGroup )
            {
                ::com::sun::star::uno::Any aAny( aXIndexAccess->getByIndex( i ) );
                if (!( aAny >>= aXShape ) )
                    return FALSE;
                aType = ByteString( String( aXShape->getShapeType() ), RTL_TEXTENCODING_UTF8 );
                aAny = ::com::sun::star::uno::Any( aXShape->queryInterface( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >*) 0 ) ));
                if (!( aAny >>= aXPropSet ) )
                    return FALSE;
            }
            else
                aType = ByteString( String( aXShape->getShapeType() ), RTL_TEXTENCODING_UTF8 );

            sal_Bool bPossibleAutoShape =
                    ( aType == "com.sun.star.drawing.PolyPolygonShape" )
                        || ( aType == "com.sun.star.drawing.PolyLineShape" )
                                || ( aType == "com.sun.star.drawing.OpenBezierShape" )
                                            || ( aType == "com.sun.star.drawing.ClosedBezierShape" );

            if ( !bPossibleAutoShape )
                return FALSE;

            bBezier = ( aType != "com.sun.star.drawing.PolyPolygonShape" )
                        && ( aType != "com.sun.star.drawing.PolyLineShape" );

            if ( !i )
            {
                ::com::sun::star::uno::Any aAny( aXPropSet->getPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "AutoShapeAdjustment" ) ) ) );
                if( !( aAny >>= aSequence ) )
                    return FALSE;
                nSequenceCount = aSequence.getLength();
                if ( nSequenceCount < 3 )
                    return FALSE;
                pPtr = aSequence.getArray();
                if ( pPtr[ nSequenceCount - 1 ] != 0x80001234 )             // this is a magic number, perhaps a ms autoshape is possible
                    return FALSE;
                if ( (sal_uInt16)pPtr[ nSequenceCount - 2 ] != nReplace )   // this is the original number of polygons the autoshape
                    return FALSE;                                           // was created to by the AutoShapeImport
                nShapeType = pPtr[ nSequenceCount - 2 ] >> 16;              // now we are setting the destination ms shapetype
            }

            ::com::sun::star::uno::Any aAny;
            if ( bBezier )
                aAny = ::com::sun::star::uno::Any( aXPropSet->getPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygonBezier" ) ) ) );
            else
                aAny = ::com::sun::star::uno::Any( aXPropSet->getPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" ) ) ) );

            if ( aAny.getValue() )
            {
                sal_Int32 nOuterSequenceCount, nInnerSequenceCount;
                ::com::sun::star::drawing::PointSequence*   pOuterSequence = NULL;
                ::com::sun::star::drawing::FlagSequence*    pOuterFlags = NULL;
                if ( bBezier )
                {
                    ::com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon =
                        ( ::com::sun::star::drawing::PolyPolygonBezierCoords* )aAny.getValue();
                    nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();
                    pOuterSequence = pSourcePolyPolygon->Coordinates.getArray();
                    pOuterFlags = pSourcePolyPolygon->Flags.getArray();
                }
                else
                {
                    ::com::sun::star::drawing::PointSequenceSequence* pSourcePolyPolygon =
                        ( ::com::sun::star::drawing::PointSequenceSequence*)aAny.getValue();
                    nOuterSequenceCount = pSourcePolyPolygon->getLength();
                    pOuterSequence = pSourcePolyPolygon->getArray();
                }
                while( nOuterSequenceCount )
                {
                    nOuterSequenceCount--;
                    ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence + nOuterSequenceCount;
                    ::com::sun::star::drawing::FlagSequence*  pInnerFlags = ( pOuterFlags ) ? pOuterFlags + nOuterSequenceCount : NULL;
                    ::com::sun::star::awt::Point* pArray = pInnerSequence->getArray();
                    ::com::sun::star::drawing::PolygonFlags* pFlags = ( pInnerFlags ) ? pInnerFlags->getArray() : NULL;
                    nInnerSequenceCount = pInnerSequence->getLength();
                    while ( nInnerSequenceCount )
                    {
                        nInnerSequenceCount--;
                        ::com::sun::star::drawing::PolygonFlags ePolyFlags
                            = ( pFlags ) ? pFlags[ nInnerSequenceCount ]
                            : ::com::sun::star::drawing::PolygonFlags_NORMAL;
                        Point aPoint( pArray[ nInnerSequenceCount ].X, pArray[ nInnerSequenceCount ].Y );

                        if ( nAngle )
                        {
                            sal_Int32 nX = aPoint.X() - aCenter.X;
                            sal_Int32 nY = aPoint.Y() - aCenter.Y;
                            Point aRotPoint( (sal_Int32)( cos( fAngle ) * nX + sin( fAngle ) * nY + 0.5 ),
                                                -(sal_Int32)( sin( fAngle ) * nX - cos( fAngle ) * nY + 0.5 ) );
                            Rectangle aRotPointRect( aRotPoint, Size( 1, 1 ) );
                            rPolyBoundRect.Union( aRotPointRect );
                        }
#ifdef __LITTLEENDIAN
                        aVal[ 0 ] = SWAPLONG( aPoint.X() - aPos.X );
                        aVal[ 1 ] = SWAPLONG( aPoint.Y() - aPos.Y );
                        aVal[ 2 ] = SWAPLONG( ePolyFlags );
#else
                        aVal[ 0 ] = aPoint.X() - aPos.X;
                        aVal[ 1 ] = aPoint.Y() - aPos.Y;
                        aVal[ 2 ] = ePolyFlags;
#endif
                        nChecksum = rtl_crc32( nChecksum, &aVal[ 0 ], 12 );
                    }
                }
            }
        }

        bIsAutoShape = pPtr[ nSequenceCount - 3 ] == nChecksum; // testing the checksum
        if ( bIsAutoShape )
        {
            if ( !nAngle )
                rPolyBoundRect = Rectangle( Point( aPos.X, aPos.Y ), Size( aSize.Width, aSize.Height ) );
            else
                rPolyBoundRect.Move( aCenter.X, aCenter.Y );
            rAdjustmentList.Clear();
            for ( i = 0; i < ( nSequenceCount - 3 ); i++ )
                rAdjustmentList.Insert( (void*)pPtr[ i ], LIST_APPEND );
        }
        if ( bIsGroup ) // the groupshape is to be removed too
            nReplace++;
    }
    catch ( ::com::sun::star::uno::Exception& )
    {
        bIsAutoShape = FALSE;
    }
    return bIsAutoShape;
}

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetEffect( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rPropSet,
                                ::com::sun::star::presentation::AnimationEffect& eEffect,
                                ::com::sun::star::presentation::AnimationEffect& eTextEffect,
                                sal_Bool& bIsSound )
{
    ::com::sun::star::uno::Any aAny;
    if ( GetPropertyValue( aAny, rPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Effect" ) ) ) )
        aAny >>= eEffect;
    else
        eEffect = ::com::sun::star::presentation::AnimationEffect_NONE;

    if ( GetPropertyValue( aAny, rPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextEffect" ) ) ) )
        aAny >>= eTextEffect;
    else
        eTextEffect = ::com::sun::star::presentation::AnimationEffect_NONE;
    if ( GetPropertyValue( aAny, rPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "SoundOn" ) ) ) )
        aAny >>= bIsSound;
    else
        bIsSound = FALSE;

    sal_Bool bHasEffect = ( ( eEffect != ::com::sun::star::presentation::AnimationEffect_NONE )
                        || ( eTextEffect != ::com::sun::star::presentation::AnimationEffect_NONE )
                        || bIsSound );
    return bHasEffect;
};

//  -----------------------------------------------------------------------

#define ADD_SHAPE( nType, nFlags )                              \
{                                                               \
    sal_uInt32 nId = mpPptEscherEx->GetShapeID();                     \
    mpPptEscherEx->AddShape( (sal_uInt32)nType, (sal_uInt32)nFlags, nId );    \
    aSolverContainer.AddShape( mXShape, nId );                  \
}

#define SHAPE_TEXT( bFill )                                     \
{                                                               \
    mnTextStyle = EPP_TEXTSTYLE_TEXT;                           \
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );         \
    ADD_SHAPE( ESCHER_ShpInst_TextBox, 0xa00 );                 \
    if ( bFill )                                                \
        aPropOpt.CreateFillProperties( mXPropSet, sal_True );   \
    ImplWriteTextBundle( aPropOpt );                            \
}

void PPTWriter::ImplWritePage( const PHLayout& rLayout, EscherSolverContainer& aSolverContainer, PageType ePageType, sal_Bool bMasterPage, int nPageNumber )
{
    sal_uInt32  nInstance, nGroups, nShapes, nShapeCount, nPer, nLastPer, nIndices, nGroupLevel, nOlePictureId;
    sal_uInt16  nEffectCount;
    ::com::sun::star::awt::Point   aTextRefPoint;

    ResetGroupTable( nShapes = mXShapes->getCount() );

    nIndices = nInstance = nLastPer = nShapeCount = nEffectCount = 0;

    sal_Bool bIsTitlePossible = TRUE;           // bei mehr als einem title geht powerpoint in die knie

    sal_uInt32  nOutlinerCount = 0;             // die gliederungsobjekte muessen dem layout entsprechen,
    sal_uInt32  nPrevTextStyle;                 // es darf nicht mehr als zwei geben

    nOlePictureId = 0;

    sal_Bool bAdditionalText = FALSE;

    SvMemoryStream* pClientTextBox = NULL;
    SvMemoryStream* pClientData = NULL;

    while( GetNextGroupEntry() )
    {
        nShapeCount++;

        nPer = ( 5 * nShapeCount ) / nShapes;
        if ( nPer != nLastPer )
        {
            nLastPer = nPer;
            sal_uInt32 nValue = mnPagesWritten * 5 + nPer;
            if ( nValue > mnStatMaxValue )
                nValue = mnStatMaxValue;
            if ( mbStatusIndicator && ( nValue > mnLatestStatValue ) )
            {
                mXStatusIndicator->setValue( nValue );
                mnLatestStatValue = nValue;
            }
        }
        nGroups = GetGroupsClosed();
        for ( sal_uInt32 i = 0; i < nGroups; i++, mpPptEscherEx->LeaveGroup() );

        if ( ImplGetShapeByIndex( GetCurrentGroupIndex(), TRUE ) )
        {
            sal_Bool bIsSound;
            ::com::sun::star::presentation::AnimationEffect eAe;
            ::com::sun::star::presentation::AnimationEffect eTe;

            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PresentationOrder" ) ) ) )
                nEffectCount = *(sal_uInt16*)mAny.getValue();

            sal_Bool bEffect = ImplGetEffect( mXPropSet, eAe, eTe, bIsSound );
            sal_Bool bIsAutoShape = sal_False;
            sal_Bool bGroup = mType == "drawing.Group";
            sal_Bool bOpenBezier   = mType == "drawing.OpenBezier";
            sal_Bool bClosedBezier = mType == "drawing.ClosedBezier";
            sal_Bool bPolyPolygon  = mType == "drawing.PolyPolygon";
            sal_Bool bPolyLine = mType == "drawing.PolyLine";


            sal_uInt32  nReplace;
            sal_uInt32  nShapeType;
            List        aAdjustmentList;
            Rectangle   aPolyBoundRect;

            const ::com::sun::star::awt::Size   aSize100thmm( mXShape->getSize() );
            const ::com::sun::star::awt::Point  aPoint100thmm( mXShape->getPosition() );
            Rectangle   aRect100thmm( Point( aPoint100thmm.X, aPoint100thmm.Y ), Size( aSize100thmm.Width, aSize100thmm.Height ) );
            EscherPropertyContainer aPropOpt( (EscherGraphicProvider&)*mpPptEscherEx, mpPicStrm, aRect100thmm );
            if ( bGroup || bOpenBezier || bClosedBezier || bPolyPolygon || bPolyLine )
                bIsAutoShape = ImplIsAutoShape( mXShape, mXPropSet, bGroup, mnAngle, nShapeType, nReplace, aAdjustmentList, aPolyBoundRect );

            if ( bIsAutoShape )
            {
                if ( bGroup )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                        aXIndexAccess( mXShape, ::com::sun::star::uno::UNO_QUERY );
                    EnterGroup( aXIndexAccess );
                    GetNextGroupEntry();
                    ImplGetShapeByIndex( GetCurrentGroupIndex(), TRUE );
                    SkipCurrentGroup();
                }
            }
            else
            {
                if ( bGroup )
                {
                    SvMemoryStream* pTmp = NULL;
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                        aXIndexAccess( mXShape, ::com::sun::star::uno::UNO_QUERY );
                    EnterGroup( aXIndexAccess );
                    if ( bEffect )
                    {
                        pTmp = new SvMemoryStream( 0x200, 0x200 );
                        ImplWriteObjectEffect( *pTmp, eAe, eTe, ++nEffectCount );
                    }
                    mpPptEscherEx->EnterGroup( &maRect, pTmp );
                    delete pTmp;
                }
                else
                {
                    sal_Bool bIsFontwork = FALSE;
                    ::com::sun::star::uno::Any aAny;
                    if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFontwork" ) ), sal_True ) )
                        aAny >>= bIsFontwork;
                    if ( bIsFontwork || ( mType == "drawing.Measure" ) || ( mType == "drawing.Caption" ) )
                    {
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "BoundRect" ) ) ) )
                        {
                            ::com::sun::star::awt::Rectangle aRect( *(::com::sun::star::awt::Rectangle*)mAny.getValue() );
                            maPosition = ImplMapPoint( ::com::sun::star::awt::Point( aRect.X, aRect.Y ) );
                            maSize = ImplMapSize( ::com::sun::star::awt::Size( aRect.Width, aRect.Height ) );
                            maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
                        }
                        mType = "drawing.dontknow";
                    }
                }
            }

            sal_uInt8 nPlaceHolderAtom = EPP_PLACEHOLDER_NONE;

            mnTextSize = 0;
            mnTextStyle = EPP_TEXTSTYLE_NORMAL;

            if ( bIsAutoShape )
            {
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( nShapeType, 0xa00 );
                ::com::sun::star::awt::Point aP( aPolyBoundRect.Left(), aPolyBoundRect.Top() );
                ::com::sun::star::awt::Size  aS( aPolyBoundRect.GetWidth(), aPolyBoundRect.GetHeight() );
                maPosition = ImplMapPoint( aP );
                maSize = ImplMapSize( aS );
                maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                else
                    mnAngle = ( 36000 - ( mnAngle % 36000 ) );
                if ( ( mnAngle > 4500 && mnAngle <= 13500 ) || ( mnAngle > 22500 && mnAngle <= 31500 ) )
                {
                    double  fWidthHalf = maRect.GetWidth() / 2;
                    double  fHeightHalf = maRect.GetHeight() / 2;

                    Point aTopLeft( (sal_Int32)( maRect.Left() + fWidthHalf - fHeightHalf ),
                                        (sal_Int32)( maRect.Top() + fHeightHalf - fWidthHalf ) );
                    Size    aNewSize( maRect.GetHeight(), maRect.GetWidth() );
                    maRect = Rectangle( aTopLeft, aNewSize );
                }
                mnAngle *= 655;
                mnAngle += 0x8000;
                mnAngle &=~0xffff;                                  // nAngle auf volle Gradzahl runden
                aPropOpt.AddOpt( ESCHER_Prop_Rotation, mnAngle );
                mnAngle = 0;

                sal_uInt16 nAdjCount;
                for ( nAdjCount = 0; nAdjCount < aAdjustmentList.Count(); nAdjCount++ )
                    aPropOpt.AddOpt( ESCHER_Prop_adjustValue + nAdjCount, (sal_uInt32)aAdjustmentList.GetObject( nAdjCount ) );

                aPropOpt.CreateFillProperties( mXPropSet, ( bPolyLine || bOpenBezier ) ? sal_False : sal_True );
                ImplWriteTextBundle( aPropOpt );
            }
            else if ( mType == "drawing.Rectangle" )
            {
                sal_Int32 nRadius = 0;
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CornerRadius" ) ) ) )
                {
                    mAny >>= nRadius;
                    nRadius = ImplMapSize( ::com::sun::star::awt::Size( nRadius, 0 ) ).Width;
                }
                if ( nRadius )
                {
                    ADD_SHAPE( ESCHER_ShpInst_RoundRectangle, 0xa00 ); // Flags: Connector | HasSpt
                    sal_Int32 nLenght = maRect.GetWidth();
                    if ( nLenght > maRect.GetHeight() )
                        nLenght = maRect.GetHeight();
                    nLenght >>= 1;
                    if ( nRadius >= nLenght )
                        nRadius = 0x2a30;                           // 0x2a30 ist PPTs maximum radius
                    else
                        nRadius = ( 0x2a30 * nRadius ) / nLenght;
                    aPropOpt.AddOpt( ESCHER_Prop_adjustValue, nRadius );
                }
                else
                {
                    ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );          // Flags: Connector | HasSpt
                }
                aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                ImplWriteTextBundle( aPropOpt );
            }
            else if ( mType == "drawing.Ellipse" )
            {
                ::com::sun::star::drawing::CircleKind  eCircleKind( ::com::sun::star::drawing::CircleKind_FULL );
                PolyStyle   ePolyKind;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CircleKind" ) ) ) )
                {
                    mAny >>= eCircleKind;
                    switch ( eCircleKind )
                    {
                        case ::com::sun::star::drawing::CircleKind_SECTION :
                        {
                            ePolyKind = POLY_PIE;
                        }
                        break;
                        case ::com::sun::star::drawing::CircleKind_ARC :
                        {
                            ePolyKind = POLY_ARC;
                        }
                        break;

                        case ::com::sun::star::drawing::CircleKind_CUT :
                        {
                            ePolyKind = POLY_CHORD;
                        }
                        break;

                        default:
                            eCircleKind = ::com::sun::star::drawing::CircleKind_FULL;
                    }
                }
                if ( eCircleKind == ::com::sun::star::drawing::CircleKind_FULL )
                {
                    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                    ADD_SHAPE( ESCHER_ShpInst_Ellipse, 0xa00 );            // Flags: Connector | HasSpt
                    aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                }
                else
                {
                    sal_Int32 nStartAngle, nEndAngle;
                    if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CircleStartAngle" ) ) ) )
                        continue;
                    nStartAngle = *( (sal_Int32*)mAny.getValue() );
                    if( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CircleEndAngle" ) ) ) )
                        continue;
                    nEndAngle = *( (sal_Int32*)mAny.getValue() );

                    ::com::sun::star::awt::Point aPoint( mXShape->getPosition() );
                    ::com::sun::star::awt::Size  aSize( mXShape->getSize() );
                    ::com::sun::star::awt::Point aStart, aEnd, aCenter;
                    Rectangle aRect( Point( aPoint.X, aPoint.Y ), Size( aSize.Width, aSize.Height ) );
                    aStart.X = (sal_Int32)( ( cos( (double)( nStartAngle * F_PI18000 ) ) * 100.0 ) );
                    aStart.Y = - (sal_Int32)( ( sin( (double)( nStartAngle * F_PI18000 ) ) * 100.0 ) );
                    aEnd.X = (sal_Int32)( ( cos( (double)( nEndAngle * F_PI18000 ) ) * 100.0 ) );
                    aEnd.Y = - (sal_Int32)( ( sin( (double)( nEndAngle * F_PI18000 ) ) * 100.0 ) );
                    aCenter.X = aPoint.X + ( aSize.Width / 2 );
                    aCenter.Y = aPoint.Y + ( aSize.Height / 2 );
                    aStart.X += aCenter.X;
                    aStart.Y += aCenter.Y;
                    aEnd.X += aCenter.X;
                    aEnd.Y += aCenter.Y;
                    Polygon aPolygon( aRect, Point( aStart.X, aStart.Y ), Point( aEnd.X, aEnd.Y ), ePolyKind );
                    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                    ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );       // Flags: Connector | HasSpt
                    ::com::sun::star::awt::Rectangle aNewRect;
                    switch ( ePolyKind )
                    {
                        case POLY_PIE :
                        case POLY_CHORD :
                        {
                            if ( aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_False, aNewRect, &aPolygon ) )
                                aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                        }
                        break;

                        case POLY_ARC :
                        {
                            if ( aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_False, aNewRect, &aPolygon ) )
                                aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                        }
                        break;
                    }
                    maRect = ImplMapRectangle( aNewRect );
                    maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                    maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                }
                ImplWriteTextBundle( aPropOpt );
            }
            else if ( mType == "drawing.Control" )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XControlShape  >
                    aXControlShape( mXShape, ::com::sun::star::uno::UNO_QUERY );
                if ( !aXControlShape.is() )
                    continue;
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                    aXControlModel( aXControlShape->getControl() );
                if ( !aXControlModel.is() )
                    continue;

                *mpExEmbed  << (sal_uInt32)( 0xf | ( EPP_ExControl << 16 ) )
                            << (sal_uInt32)0;               // Size of this container

                sal_uInt32 nSize, nOldPos = mpExEmbed->Tell();

                sal_uInt32 nPageId = nPageNumber;
                if ( ePageType == MASTER )
                    nPageId |= 0x80000000;
                else
                    nPageId += 0x100;
                *mpExEmbed  << (sal_uInt32)( EPP_ExControlAtom << 16 )
                            << (sal_uInt32)4
                            << nPageId;
                PPTExOleObjEntry* pEntry = new PPTExOleObjEntry( OCX_CONTROL, String(), mpExEmbed->Tell() );
                pEntry->xControlModel = aXControlModel;
                maExOleObj.Insert( pEntry );

                mnExEmbed++;

                *mpExEmbed  << (sal_uInt32)( 1 | ( EPP_ExOleObjAtom << 16 ) )
                            << (sal_uInt32)24
                            << (sal_uInt32)1
                            << (sal_uInt32)2
                            << (sal_uInt32)mnExEmbed
                            << (sal_uInt32)0
                            << (sal_uInt32)4    // index to the persist table
                            << (sal_uInt32)0x0012de00;


                ::com::sun::star::awt::Size     aSize;
                String          aControlName;
                SvStorageRef    xTemp( new SvStorage( new SvMemoryStream(), TRUE ) );
                if ( SvxMSConvertOCXControls::WriteOCXStream( xTemp, aXControlModel, aSize, aControlName ) )
                {
                    String  aUserName( xTemp->GetUserName() );
                    String  aOleIdentifier;
                    if ( aUserName.Len() )
                    {
                        SvStorageStreamRef xCompObj = xTemp->OpenStream(
                            String( RTL_CONSTASCII_USTRINGPARAM( "\1CompObj" ) ),
                                STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL );
                        xCompObj->Seek( STREAM_SEEK_TO_END );
                        sal_uInt32  nStreamLen = xCompObj->Tell();
                        xCompObj->Seek( 0 );
                        sal_Int16   nVersion, nByteOrder;
                        sal_Int32   nWinVersion, nVal, nStringLen;
                        *xCompObj   >> nVersion
                                    >> nByteOrder
                                    >> nWinVersion
                                    >> nVal;
                        xCompObj->SeekRel( 16 );    // skipping clsid
                        *xCompObj   >> nStringLen;
                        if ( ( xCompObj->Tell() + nStringLen ) < nStreamLen )
                        {
                            xCompObj->SeekRel( nStringLen );        // now skipping the UserName;
                            *xCompObj >> nStringLen;
                            if ( ( xCompObj->Tell() + nStringLen ) < nStreamLen )
                            {
                                xCompObj->SeekRel( nStringLen );    // now skipping the clipboard formatname
                                *xCompObj   >> nStringLen;
                                if ( ( nStringLen > 1 ) && ( ( xCompObj->Tell() + nStringLen ) < nStreamLen ) )
                                {   // i think that the OleIdentifier will follow
                                    ByteString aTemp;
                                    sal_Char* p = aTemp.AllocBuffer( nStringLen - 1 );
                                    xCompObj->Read( p, nStringLen - 1 );
                                    aOleIdentifier = String( aTemp, gsl_getSystemTextEncoding() );
                                }
                            }
                        }
                    }
                    if ( aControlName.Len() )
                         ImplWriteCString( *mpExEmbed, aControlName, 1 );
                    if ( aOleIdentifier.Len() )
                        ImplWriteCString( *mpExEmbed, aOleIdentifier, 2 );
                    if ( aUserName.Len() )
                        ImplWriteCString( *mpExEmbed, aUserName, 3 );
                }
                nSize = mpExEmbed->Tell() - nOldPos;
                mpExEmbed->Seek( nOldPos - 4 );
                *mpExEmbed << nSize;
                mpExEmbed->Seek( STREAM_SEEK_TO_END );
                nOlePictureId = mnExEmbed;

                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                sal_uInt32 nSpFlags = SHAPEFLAG_HAVESPT | SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_OLESHAPE;
                ADD_SHAPE( ESCHER_ShpInst_HostControl, nSpFlags );
                if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False  ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                aPropOpt.AddOpt( ESCHER_Prop_pictureId, mnExEmbed );
                aPropOpt.AddOpt( ESCHER_Prop_pictureActive, 0x10000 );

                if ( aControlName.Len() )
                {
                    sal_uInt16 i, nBufSize;
                    nBufSize = ( aControlName.Len() + 1 ) << 1;
                    sal_uInt8* pBuf = new sal_uInt8[ nBufSize ];
                    sal_uInt8* pTmp = pBuf;
                    for ( i = 0; i < aControlName.Len(); i++ )
                    {
                        sal_Unicode nUnicode = aControlName.GetChar( i );
                        *pTmp++ = (sal_uInt8)nUnicode;
                        *pTmp++ = (sal_uInt8)( nUnicode >> 8 );
                    }
                    *pTmp++ = 0;
                    *pTmp = 0;
                    aPropOpt.AddOpt( ESCHER_Prop_wzName, TRUE, nBufSize, pBuf, nBufSize );
                }
            }
            else if ( mType == "drawing.Connector" )
            {
                sal_uInt16 nSpType, nSpFlags;
                ::com::sun::star::awt::Rectangle aNewRect;
                if ( aPropOpt.CreateConnectorProperties( mXShape, aSolverContainer, aNewRect, nSpType, nSpFlags ) == sal_False )
                    continue;

                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );

                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( nSpType, nSpFlags );
            }
            else if ( mType == "drawing.Measure" )
            {
                continue;
            }
            else if ( mType == "drawing.Line" )
            {
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_LINE, sal_False, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                if ( ImplGetText() )
                {
                    aTextRefPoint = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                    mnTextSize = 0;
                    bAdditionalText = TRUE;
                    mpPptEscherEx->EnterGroup( &maRect );
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                sal_uInt32 nFlags = 0xa00;                                  // Flags: Connector | HasSpt
                if ( maRect.Top() > maRect.Bottom() )
                    nFlags |= 0x80;                                         // Flags: VertMirror
                if ( maRect.Left() > maRect.Right() )
                    nFlags |= 0x40;                                         // Flags: HorzMirror

                ADD_SHAPE( ESCHER_ShpInst_Line, nFlags );
                aPropOpt.AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
                aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                mnAngle = 0;
            }
            else if ( bPolyPolygon )
            {
                if ( ImplGetText() )
                {
                    mpPptEscherEx->EnterGroup( NULL );
                    nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = TRUE;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );            // Flags: Connector | HasSpt
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_False, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                mnAngle = 0;
            }
            else if ( bPolyLine )
            {
                if ( ImplGetText() )
                {
                    mpPptEscherEx->EnterGroup( NULL );
                    nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = TRUE;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );            // Flags: Connector | HasSpt
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_False, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                mnAngle = 0;
            }
            else if ( bOpenBezier )
            {
                if ( ImplGetText() )
                {
                    mpPptEscherEx->EnterGroup( NULL );
                    nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = TRUE;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );            // Flags: Connector | HasSpt
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_True, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                mnAngle = 0;
            }
            else if ( bClosedBezier )
            {
                if ( ImplGetText() )
                {
                    mpPptEscherEx->EnterGroup( NULL );
                    nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = TRUE;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );            // Flags: Connector | HasSpt
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_True, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                mnAngle = 0;
            }
            else if ( ( mType == "drawing.GraphicObject" ) || ( mType == "presentation.GraphicObject" ) )
            {
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );

                // ein GraphicObject kann auch ein ClickMe Element sein
                if ( mbEmptyPresObj && ( ePageType == NORMAL ) )
                {
                    nPlaceHolderAtom = rLayout.nUsedObjectPlaceHolder;
                    ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );           // Flags: HaveAnchor | HaveMaster
                    aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
                }
                else
                {
                    mXText = ::com::sun::star::uno::Reference<
                        ::com::sun::star::text::XSimpleText >
                            ( mXShape, ::com::sun::star::uno::UNO_QUERY );

                    if ( mXText.is() )
                        mnTextSize = mXText->getString().len();

                    if ( mnTextSize )                                       // graphic object oder Flachenfuellung
                    {
                        ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );       // Flags: Connector | HasSpt

                        if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), sal_True ) )
                        {
                            aPropOpt.AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
                            aPropOpt.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
                            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
                            aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x8000000 );
                            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
                            ImplWriteTextBundle( aPropOpt );
                        }
                    }
                    else
                    {
                        ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );
                        if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), sal_False, sal_True ) )
                            aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                    }
                }
            }
            else if ( ( mType == "drawing.Text" ) || ( mType == "presentation.Notes" ) )
            {
                if ( ( ePageType == NOTICE ) && mbPresObj )
                {
                    if ( bMasterPage )
                        nPlaceHolderAtom = EPP_PLACEHOLDER_MASTERDATE;  // be sure to correct all defines, they are not in the right order,
                    else                                                // cause this is not a masterdate,´it is realy a masternotesbodyimage
                        nPlaceHolderAtom = EPP_PLACEHOLDER_NOTESBODY;
                }
                SHAPE_TEXT( TRUE );
            }
            else if ( mType == "presentation.TitleText" )
            {
                if ( mbPresObj )
                {
                    if ( ( ePageType == NOTICE ) && mbEmptyPresObj )
                    {
                        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                        nPlaceHolderAtom = EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE;
                        ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x200 );
                        aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                    }
                    else if ( rLayout.bTitlePossible && bIsTitlePossible )
                    {
                        bIsTitlePossible = FALSE;

                        if ( ePageType == MASTER )
                            continue;

                        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                        mnTextStyle = EPP_TEXTSTYLE_TITLE;
                        nPlaceHolderAtom = rLayout.nTypeOfTitle;
                        ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );          // Flags: HaveAnchor | HaveMaster
                        aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterTitle );
                        aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                        ImplWriteTextBundle( aPropOpt, sal_True, sal_True );
                        if ( mbEmptyPresObj )
                        {
                            sal_uInt32 nNoLineDrawDash = 0;
                            aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
                            nNoLineDrawDash |= 0x10001;
                            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
                        }
                    }
                    else
                        mbPresObj = FALSE;
                }
                if ( !mbPresObj )
                {
                    mType = "drawing.Text";
                    SHAPE_TEXT( TRUE );
                }
            }
            else if ( ( mType == "presentation.Outliner" ) || ( mType == "presentation.Subtitle" ) )
            {
                if ( mbPresObj )
                {
                    nOutlinerCount++;
                    if ( rLayout.bOutlinerPossible && ( nOutlinerCount == 1 ) ||
                        ( ( rLayout.bSecOutlinerPossible && ( nOutlinerCount == 2 ) )
                            && ( nPrevTextStyle == EPP_TEXTSTYLE_BODY ) ) )
                    {
                        if ( ePageType == MASTER )
                        {
                            nPrevTextStyle = EPP_TEXTSTYLE_TITLE;
                            continue;
                        }
                        mnTextStyle = EPP_TEXTSTYLE_BODY;
                        nPlaceHolderAtom = rLayout.nTypeOfOutliner;
                        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                        ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );          // Flags: HaveAnchor | HaveMaster
                        aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
                        aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                        ImplWriteTextBundle( aPropOpt, sal_True, sal_True );
                        if ( mbEmptyPresObj )
                        {
                            sal_uInt32 nNoLineDrawDash = 0;
                            aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
                            nNoLineDrawDash |= 0x10001;
                            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
                        }
                    }
                    else
                        mbPresObj = FALSE;
                }
                if ( !mbPresObj )
                {
                    mType = "drawing.Text";
                    SHAPE_TEXT( TRUE );
                }
            }
            else if ( ( mType == "drawing.Page" ) || ( mType == "presentation.Page" ) )
            {
                nPlaceHolderAtom = EPP_PLACEHOLDER_GENERICTEXTOBJECT;
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );
                aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x40004 );
                aPropOpt.AddOpt( ESCHER_Prop_fFillOK, 0x100001 );
                aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110011 );
                aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90008 );
                aPropOpt.AddOpt( ESCHER_Prop_fshadowObscured, 0x10001 );
            }
            else if ( mType == "drawing.Frame" )
            {
                continue;
            }
            else if ( ( mType == "drawing.OLE2" ) || ( mType == "presentation.OLE2" )
                        || ( mType == "presentation.Chart" ) || ( mType == "presentation.Table" )
                            || ( mType == "presentation.OrgChart" ) )
            {
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                if ( mbEmptyPresObj && ( ePageType == NORMAL ) )
                {
                    nPlaceHolderAtom = rLayout.nUsedObjectPlaceHolder;
                    ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );              // Flags: HaveAnchor | HaveMaster
                    aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
                }
                else
                {
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "LinkDisplayName" ) ) ) )
                    {
                        String  aString( *(::rtl::OUString*)mAny.getValue() );
                        if ( aString.Len() )
                        {
                            SvStorageRef xSrcStor = mXSource->OpenStorage( aString, STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL );
                            if ( xSrcStor.Is() )
                            {
                                SvStorageStreamRef xSrcTst = xSrcStor->OpenStream(
                                    String( RTL_CONSTASCII_USTRINGPARAM( "\1Ole" ) ),
                                        STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL );
                                sal_uInt8 aTestA[ 10 ];
                                if ( sizeof( aTestA ) == xSrcTst->Read( aTestA, sizeof( aTestA ) ) )
                                {
/*
                                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CLSID" ) ) ) )
                                    {
                                        String aCLSID( *( ::rtl::OUString*)mAny.getValue() );
                                    }
*/
                                    *mpExEmbed  << (sal_uInt32)( 0xf | ( EPP_ExEmbed << 16 ) )
                                                << (sal_uInt32)0;               // Size of this container

                                    sal_uInt32 nSize, nOldPos = mpExEmbed->Tell();

                                    *mpExEmbed  << (sal_uInt32)( EPP_ExEmbedAtom << 16 )
                                                << (sal_uInt32)8
                                                << (sal_uInt32)0    // follow colorscheme : 0->do not follow
                                                                    //                      1->follow collorscheme
                                                                    //                      2->follow text and background scheme
                                                << (sal_uInt8)1     // (bool)set if embedded server can not be locked
                                                << (sal_uInt8)0     // (bool)do not need to send dimension
                                                << (sal_uInt8)0     // (bool)is object a world table
                                                << (sal_uInt8)0;    // pad byte

                                    maExOleObj.Insert( new PPTExOleObjEntry( NORMAL_OLE_OBJECT, aString, mpExEmbed->Tell() ) );

                                    mnExEmbed++;

                                    *mpExEmbed  << (sal_uInt32)( 1 | ( EPP_ExOleObjAtom << 16 ) )
                                                << (sal_uInt32)24
                                                << (sal_uInt32)1
                                                << (sal_uInt32)0
                                                << (sal_uInt32)mnExEmbed    // index to the persist table
                                                << (sal_uInt32)0            // subtype
                                                << (sal_uInt32)0
                                                << (sal_uInt32)0x0012b600;

//                                  ImplWriteCString( *mpExEmbed, "Photo Editor Photo", 1 );
//                                  ImplWriteCString( *mpExEmbed, "MSPhotoEd.3", 2 );
//                                  ImplWriteCString( *mpExEmbed, "Microsoft Photo Editor 3.0 Photo", 3 );

                                    nSize = mpExEmbed->Tell() - nOldPos;
                                    mpExEmbed->Seek( nOldPos - 4 );
                                    *mpExEmbed << nSize;
                                    mpExEmbed->Seek( STREAM_SEEK_TO_END );
                                    nOlePictureId = mnExEmbed;
                                }
                            }
                        }
                    }
                    sal_uInt32 nSpFlags = 0xa00;
                    if ( nOlePictureId )
                        nSpFlags |= 0x10;
                    ADD_SHAPE( ESCHER_ShpInst_PictureFrame, nSpFlags );
                    if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False ) )
                        aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                    if ( nOlePictureId )
                        aPropOpt.AddOpt( ESCHER_Prop_pictureId, nOlePictureId );
                }
            }
            else if ( ( (sal_Char)'3' == mType.GetChar( 8 ) ) && ( (char)'D' == mType.GetChar( 9 ) ) )  // drawing.3D
            {
                // SceneObject, CubeObject, SphereObject, LatheObject, ExtrudeObject, PolygonObject
                if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ) ) )
                    continue;

                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );

                if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ), sal_False ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
            }
            else if ( mType == "drawing.dontknow" )
            {
                mnAngle = 0;
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );
                if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
            }
            else
            {
                continue;
            }
            aPropOpt.CreateShadowProperties( mXPropSet );
            maRect.Justify();

            if ( mnAngle )
                ImplFlipBoundingBox( aPropOpt );

            aPropOpt.Commit( *mpStrm );
            mpPptEscherEx->AddClientAnchor( maRect );

            ::com::sun::star::presentation::ClickAction eCa = ::com::sun::star::presentation::ClickAction_NONE;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "OnClick" ) ) ) )
                mAny >>= eCa;

            sal_Int32 nPlacementID = -1;

            sal_Bool bClientData = ( bEffect || ( eCa != ::com::sun::star::presentation::ClickAction_NONE ) ||
                                        nPlaceHolderAtom || nOlePictureId );
            if ( bClientData )
            {
                if ( nPlaceHolderAtom )
                {
                    if ( ( mnTextStyle == EPP_TEXTSTYLE_TITLE ) || ( mnTextStyle == EPP_TEXTSTYLE_BODY ) )
                        nPlacementID = nIndices++;
                    else
                    {
                        switch ( nPlaceHolderAtom )
                        {
                            default :
                            {
                                if ( nPlaceHolderAtom < 19 )
                                    break;
                            }
                            case EPP_PLACEHOLDER_NOTESBODY :
                            case EPP_PLACEHOLDER_MASTERDATE :
                            case EPP_PLACEHOLDER_GENERICTEXTOBJECT :
                            case EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE :
                                nPlacementID = nIndices++;
                        }
                    }
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );

                    *pClientData << (sal_uInt32)( EPP_OEPlaceholderAtom << 16 ) << (sal_uInt32)8
                                 << nPlacementID                // PlacementID
                                 << (sal_uInt8)nPlaceHolderAtom // PlaceHolderID
                                 << (sal_uInt8)0                // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
                                 << (sal_uInt16)0;              // padword
                }
                if ( nOlePictureId )
                {
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );

                    *pClientData << (sal_uInt32)( EPP_ExObjRefAtom << 16 ) << (sal_uInt32)4
                                 << nOlePictureId;
                    nOlePictureId = 0;
                }
                if ( bEffect )
                {
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );

                    ImplWriteObjectEffect( *pClientData, eAe, eTe, ++nEffectCount );
                }

                if ( eCa != ::com::sun::star::presentation::ClickAction_NONE )
                {
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );
                    ImplWriteClickAction( *pClientData, eCa );
                }
            }
            if ( ( mnTextStyle == EPP_TEXTSTYLE_TITLE ) || ( mnTextStyle == EPP_TEXTSTYLE_BODY ) )
            {
                if ( !pClientTextBox )
                    pClientTextBox = new SvMemoryStream( 0x200, 0x200 );

                *pClientTextBox << (sal_uInt32)( EPP_OutlineTextRefAtom << 16 ) << (sal_uInt32)4
                                << nPlacementID;

                if ( mbEmptyPresObj == FALSE )
                {
                    if ( ( ePageType == NORMAL ) && ( bMasterPage == FALSE ) )
                    {   // try to allocate the textruleratom
                        TextRuleEntry*  pTextRule = (TextRuleEntry*)maTextRuleList.GetCurObject();
                        while ( pTextRule )
                        {
                            int nRulePage = pTextRule->nPageNumber;
                            if ( nRulePage > nPageNumber )
                                break;
                            else if ( nRulePage < nPageNumber )
                                pTextRule = (TextRuleEntry*)maTextRuleList.Next();
                            else
                            {
                                SvMemoryStream* pOut = pTextRule->pOut;
                                if ( pOut )
                                {
                                    pClientTextBox->Write( pOut->GetData(), pOut->Tell() );
                                    delete pOut, pTextRule->pOut = NULL;
                                }
                                maTextRuleList.Next();
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                if ( mnTextSize || ( nPlaceHolderAtom == EPP_PLACEHOLDER_MASTERDATE ) || ( nPlaceHolderAtom == EPP_PLACEHOLDER_NOTESBODY ) )
                {
                    int nInstance;
                    if ( ( nPlaceHolderAtom == EPP_PLACEHOLDER_MASTERDATE ) || ( nPlaceHolderAtom == EPP_PLACEHOLDER_NOTESBODY ) )
                        nInstance = 2;
                    else
                        nInstance = EPP_TEXTTYPE_Other;     // Text in a Shape

                    if ( !pClientTextBox )
                        pClientTextBox = new SvMemoryStream( 0x200, 0x200 );

                    SvMemoryStream  aExtBu( 0x200, 0x200 );
                    ImplWriteTextStyleAtom( *pClientTextBox, nInstance, 0, NULL, aExtBu );
                    if ( aExtBu.Tell() )
                    {
                        if ( !pClientData )
                            pClientData = new SvMemoryStream( 0x200, 0x200 );
                        ImplProgTagContainer( pClientData, &aExtBu );
                    }
                }
                else if ( nPlaceHolderAtom >= 19 )
                {
                    if ( !pClientTextBox )
                        pClientTextBox = new SvMemoryStream( 12 );

                    *pClientTextBox << (sal_uInt32)( EPP_TextHeaderAtom << 16 ) << (sal_uInt32)4
                                    << (sal_uInt32)7;
                }
            }
            if ( pClientData )
            {
                *mpStrm << (sal_uInt32)( ( ESCHER_ClientData << 16 ) | 0xf )
                        << (sal_uInt32)pClientData->Tell();

                mpStrm->Write( pClientData->GetData(), pClientData->Tell() );
                delete pClientData, pClientData = NULL;
            }
            if ( pClientTextBox )
            {
                *mpStrm << (sal_uInt32)( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                        << (sal_uInt32)pClientTextBox->Tell();

                mpStrm->Write( pClientTextBox->GetData(), pClientTextBox->Tell() );
                delete pClientTextBox, pClientTextBox = NULL;
            }
            mpPptEscherEx->CloseContainer();      // ESCHER_SpContainer
        }
        nPrevTextStyle = mnTextStyle;

        if ( bAdditionalText )
        {
            bAdditionalText = FALSE;

            ::com::sun::star::uno::Any  aAny;
            EscherPropertyContainer     aPropOpt;
            mnAngle = ( PropValue::GetPropertyValue( aAny,
                mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ), sal_True ) )
                    ? *((sal_Int32*)aAny.getValue() )
                    : 0;

            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            if ( mType == "drawing.Line" )
            {
                double fDist = hypot( maRect.GetWidth(), maRect.GetHeight() );
                maRect = Rectangle( Point( aTextRefPoint.X, aTextRefPoint.Y ),
                                        Point( (sal_Int32)( aTextRefPoint.X + fDist ), aTextRefPoint.Y - 1 ) );
                SHAPE_TEXT( FALSE );
                aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x60006 );        // Size Shape To Fit Text
                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                if ( mnAngle )
                    ImplFlipBoundingBox( aPropOpt );
            }
            else
            {
                SHAPE_TEXT( FALSE );

                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                else
                    mnAngle = ( 36000 - ( mnAngle % 36000 ) );

                mnAngle *= 655;
                mnAngle += 0x8000;
                mnAngle &=~0xffff;  // nAngle auf volle Gradzahl runden
                aPropOpt.AddOpt( ESCHER_Prop_Rotation, mnAngle );
                mpPptEscherEx->SetGroupSnapRect( nGroupLevel, maRect );
                mpPptEscherEx->SetGroupLogicRect( nGroupLevel, maRect );
            }
            aPropOpt.Commit( *mpStrm );

            mpPptEscherEx->AddClientAnchor( maRect );
            if ( !pClientTextBox )
                pClientTextBox = new SvMemoryStream( 0x200, 0x200 );

            SvMemoryStream  aExtBu( 0x200, 0x200 );
            ImplWriteTextStyleAtom( *pClientTextBox, EPP_TEXTTYPE_Other, 0, NULL, aExtBu );

            *mpStrm << (sal_uInt32)( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                    << (sal_uInt32)pClientTextBox->Tell();

            mpStrm->Write( pClientTextBox->GetData(), pClientTextBox->Tell() );
            delete pClientTextBox, pClientTextBox = NULL;

            mpPptEscherEx->CloseContainer();  // ESCHER_SpContainer
            mpPptEscherEx->LeaveGroup();
        }
    }
    ClearGroupTable();                              // gruppierungen wegschreiben, sofern noch irgendwelche offen sind, was eigendlich nicht sein sollte
    nGroups = GetGroupsClosed();
    for ( sal_uInt32 i = 0; i < nGroups; i++, mpPptEscherEx->LeaveGroup() );
    mnPagesWritten++;
}

//  -----------------------------------------------------------------------

::com::sun::star::awt::Point PPTWriter::ImplMapPoint( const ::com::sun::star::awt::Point& rPoint )
{
    Point aRet( OutputDevice::LogicToLogic( Point( rPoint.X, rPoint.Y ), maMapModeSrc, maMapModeDest ) );
    return ::com::sun::star::awt::Point( aRet.X(), aRet.Y() );
}

//  -----------------------------------------------------------------------

::com::sun::star::awt::Size PPTWriter::ImplMapSize( const ::com::sun::star::awt::Size& rSize )
{
    Size aRetSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );

    if ( !aRetSize.Width() )
        aRetSize.Width()++;
    if ( !aRetSize.Height() )
        aRetSize.Height()++;
    return ::com::sun::star::awt::Size( aRetSize.Width(), aRetSize.Height() );
}

//  -----------------------------------------------------------------------

Rectangle PPTWriter::ImplMapRectangle( const ::com::sun::star::awt::Rectangle& rRect )
{
    ::com::sun::star::awt::Point    aPoint( rRect.X, rRect.Y );
    ::com::sun::star::awt::Size     aSize( rRect.Width, rRect.Height );
    ::com::sun::star::awt::Point    aP( ImplMapPoint( aPoint ) );
    ::com::sun::star::awt::Size     aS( ImplMapSize( aSize ) );
    return Rectangle( Point( aP.X, aP.Y ), Size( aS.Width, aS.Height ) );
}

//  -----------------------------------------------------------------------
