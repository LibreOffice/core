/*************************************************************************
 *
 *  $RCSfile: vclxfont.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:09 $
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


#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>
#include <rtl/ustring.h>

#include <vcl/outdev.hxx>

//  ----------------------------------------------------
//  class VCLXFont
//  ----------------------------------------------------
VCLXFont::VCLXFont()
{
    mpFontMetric = NULL;
}

VCLXFont::~VCLXFont()
{
    delete mpFontMetric;
}

void VCLXFont::Init( ::com::sun::star::awt::XDevice& rxDev, const Font& rFont )
{
    mxDevice = &rxDev;

    delete mpFontMetric;
    mpFontMetric = NULL;

    maFont = rFont;
}

BOOL VCLXFont::ImplAssertValidFontMetric()
{
    if ( !mpFontMetric && mxDevice.is() )
    {
        OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
        if ( pOutDev )
        {
            Font aOldFont = pOutDev->GetFont();
            pOutDev->SetFont( maFont );
            mpFontMetric = new FontMetric( pOutDev->GetFontMetric() );
            pOutDev->SetFont( aOldFont );
        }
    }
    return mpFontMetric ? TRUE : FALSE;
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXFont::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XFont*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XUnoTunnel*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXFont )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFont )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont>* ) NULL )
IMPL_XTYPEPROVIDER_END


::com::sun::star::awt::FontDescriptor VCLXFont::getFontDescriptor(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return VCLUnoHelper::CreateFontDescriptor( maFont );

}

::com::sun::star::awt::SimpleFontMetric VCLXFont::getFontMetric(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::awt::SimpleFontMetric aFM;
    if ( ImplAssertValidFontMetric() )
        aFM = VCLUnoHelper::CreateFontMetric( *mpFontMetric );
    return aFM;
}

sal_Int16 VCLXFont::getCharWidth( sal_Unicode c ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int16 nRet = -1;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );

        nRet = pOutDev->GetTextWidth( c );

        pOutDev->SetFont( aOldFont );
    }
    return nRet;
}

::com::sun::star::uno::Sequence< sal_Int16 > VCLXFont::getCharWidths( sal_Unicode nFirst, sal_Unicode nLast ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence<sal_Int16> aSeq;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );

        sal_Int16 nCount = nLast-nFirst + 1;
        aSeq = ::com::sun::star::uno::Sequence<sal_Int16>( nCount );
        for ( USHORT n = 0; n < nCount; n++ )
        {
            aSeq.getArray()[n] = pOutDev->GetTextWidth( nFirst+n );
        }

        pOutDev->SetFont( aOldFont );
    }
    return aSeq;
}

sal_Int32 VCLXFont::getStringWidth( const ::rtl::OUString& str ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nRet = -1;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );
        nRet = pOutDev->GetTextWidth( str );
        pOutDev->SetFont( aOldFont );
    }
    return nRet;
}

sal_Int32 VCLXFont::getStringWidthArray( const ::rtl::OUString& str, ::com::sun::star::uno::Sequence< sal_Int32 >& rDXArray ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nRet = -1;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );
        rDXArray = ::com::sun::star::uno::Sequence<sal_Int32>( str.getLength() );
        nRet = pOutDev->GetTextArray( str, rDXArray.getArray() );
        pOutDev->SetFont( aOldFont );
    }
    return nRet;
}

void VCLXFont::getKernPairs( ::com::sun::star::uno::Sequence< sal_Unicode >& rnChars1, ::com::sun::star::uno::Sequence< sal_Unicode >& rnChars2, ::com::sun::star::uno::Sequence< sal_Int16 >& rnKerns ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );

        ULONG nPairs = pOutDev->GetKerningPairCount();
        if ( nPairs )
        {
            KerningPair* pData = new KerningPair[ nPairs ];
            pOutDev->GetKerningPairs( nPairs, pData );

            rnChars1 = ::com::sun::star::uno::Sequence<sal_Unicode>( nPairs );
            rnChars2 = ::com::sun::star::uno::Sequence<sal_Unicode>( nPairs );
            rnKerns = ::com::sun::star::uno::Sequence<sal_Int16>( nPairs );

            sal_Unicode* pChars1 = rnChars1.getArray();
            sal_Unicode* pChars2 = rnChars2.getArray();
            sal_Int16* pKerns = rnKerns.getArray();

            for ( ULONG n = 0; n < nPairs; n++ )
            {
                pChars1[n] = pData[n].nChar1;
                pChars2[n] = pData[n].nChar2;
                pKerns[n] = pData[n].nKern;
            }


            delete pData;
        }
        pOutDev->SetFont( aOldFont );
    }
}



