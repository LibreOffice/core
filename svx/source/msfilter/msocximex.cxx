/*************************************************************************
 *
 *  $RCSfile: msocximex.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-16 16:14:31 $
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

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HDL_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _MSOCXIMEX_HXX
#include <msocximex.hxx>
#endif

#ifndef C2S
#define C2S(cChar)  String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))
#endif

using namespace ::com::sun::star;
#if 0
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
#endif
using namespace ::rtl;
using namespace cppu;


#define WW8_ASCII2STR(s) String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(s))


static char sWW8_form[] = "WW-Standard";


sal_uInt32 OCX_Control::pColor[25] = {
0xC0C0C0, 0x008080, 0x000080, 0x808080, 0xC0C0C0, 0xFFFFFF, 0x000000,
0x000000, 0x000000, 0xFFFFFF, 0xC0C0C0, 0xC0C0C0, 0x808080, 0x000080,
0xFFFFFF, 0xC0C0C0, 0x808080, 0x808080, 0x000000, 0xC0C0C0, 0xFFFFFF,
0x000000, 0xC0C0C0, 0x000000, 0xFFFFC0 };

sal_uInt8 __READONLY_DATA OCX_Control::aObjInfo[4] = {
0x00, 0x12, 0x03, 0x00, };


void Align(SvStorageStream *pS,int nAmount,BOOL bFill=FALSE)
{
    if (pS->Tell()%nAmount)
    {
        if (bFill == FALSE)
            pS->SeekRel(nAmount-(pS->Tell()%nAmount));
        else
        {
        int nLen = nAmount-(pS->Tell()%nAmount);
        for(int i=0;i<nLen;i++)
            *pS << sal_uInt8(0x00);
        }
    }
}

sal_uInt16 OCX_Control::nStandardId(0x0200);
sal_uInt16 OCX_FontData::nStandardId(0x0200);

sal_uInt32 OCX_Control::ImportColor(sal_uInt32 nColor) const
{
    sal_uInt8 nUpper = (sal_uInt8)( nColor >> 24 );
    if (nUpper & 0x80) //Palette color, should be switch on bottom 24 bits
    {
        /*Might as well use my systems ones in the absence of any other ideas*/
        nColor = nColor&0x00FFFFFF;
        DBG_ASSERT (nColor <= 24,"Unknown Palette Index");
        if (nColor > 24)
            nColor = 0xFFFFFF;
        else
            nColor = pColor[nColor];
    }
    else
    {
        //Stored in bgr! rather than rgb
        nColor = SwapColor(nColor);
    }
    return nColor;
}

sal_Int16 OCX_FontData::ImportAlign(sal_uInt8 nJustification) const
{
    sal_Int16 nRet;
    switch (nJustification)
    {
    default:
    case 1:
        nRet = 0;
        break;
    case 2:
        nRet = 2;
        break;
    case 3:
        nRet = 1;
        break;
    }
    return nRet;
}

sal_uInt8 OCX_FontData::ExportAlign(sal_Int16 nAlign) const
{
    sal_Int8 nRet;
    switch (nAlign)
    {
    default:
    case 0:
        nRet = 1;
        break;
    case 2:
        nRet = 2;
        break;
    case 1:
        nRet = 3;
        break;
    }
    return nRet;
}

sal_uInt32 OCX_Control::SwapColor(sal_uInt32 nColor) const
{
    sal_uInt8
        r(static_cast<sal_uInt8>(nColor&0xFF)),
        g(static_cast<sal_uInt8>(((nColor)>>8)&0xFF)),
        b(static_cast<sal_uInt8>((nColor>>16)&0xFF));
    nColor = (r<<16) + (g<<8) + b;
    return nColor;
}

sal_uInt32 OCX_Control::ExportColor(sal_uInt32 nColor) const
{
    sal_uInt8 nUpper = (sal_uInt8)( nColor >> 24 );
    if (nUpper & 0x80) //Palette color, should be switch on bottom 24 bits
    {
        /*Might as well use my systems ones in the absence of any other ideas*/
        nColor = nColor&0x00FFFFFF;
        DBG_ASSERT (nColor <= 24,"Unknown Palette Index");
        if (nColor > 24)
            nColor = 0xFFFFFF;
        else
            nColor = pColor[nColor];
    }
    else
    {
        //Stored in bgr! rather than rgb
        nColor = SwapColor(nColor);
    }
    return nColor;
}


sal_Int16 OCX_Control::ImportBorder(sal_uInt16 nSpecialEffect,
    sal_uInt16 nBorderStyle) const
{
    if ((nSpecialEffect == 0) && (nBorderStyle == 0))
        return 0;   //No Border
    else if ((nSpecialEffect == 0) && (nBorderStyle == 1))
        return 2;   //Flat Border
    return 1;   //3D Border
}

sal_uInt8 OCX_Control::ExportBorder(sal_uInt16 nBorder,sal_uInt8 &rBorderStyle)
    const
{
    sal_uInt8 nRet;
    switch(nBorder)
    {
        case 0:
            nRet = rBorderStyle = 0;
            break;
        default:
        case 1:
            nRet = 2;
            rBorderStyle = 1;
            break;
        case 2:
            nRet = 3;
            rBorderStyle = 1;
            break;
    }
    return nRet;
}



const uno::Reference< drawing::XDrawPage >&
    SvxMSConvertOCXControls::GetDrawPage()
{
    if( !xDrawPage.is() && pDocSh )
    {
        uno::Reference< drawing::XDrawPageSupplier > xTxtDoc(pDocSh->GetModel(),
            uno::UNO_QUERY);
        DBG_ASSERT(xTxtDoc.is(),"XDrawPageSupplier nicht vom XModel erhalten");
        xDrawPage = xTxtDoc->getDrawPage();
        DBG_ASSERT( xDrawPage.is(), "XDrawPage nicht erhalten" );
    }

    return xDrawPage;
}


const uno::Reference< lang::XMultiServiceFactory >&
    SvxMSConvertOCXControls::GetServiceFactory()
{
    if( !xServiceFactory.is() && pDocSh )
    {
        xServiceFactory = uno::Reference< lang::XMultiServiceFactory >
            (pDocSh->GetBaseModel(), uno::UNO_QUERY);
        DBG_ASSERT( xServiceFactory.is(),
                "XMultiServiceFactory nicht vom Model erhalten" );
    }

    return xServiceFactory;
}

const uno::Reference< drawing::XShapes >& SvxMSConvertOCXControls::GetShapes()
{
    if( !xShapes.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {

            xShapes = uno::Reference< drawing::XShapes >(xDrawPage,
                uno::UNO_QUERY);
            DBG_ASSERT( xShapes.is(), "XShapes nicht vom XDrawPage erhalten" );
        }
    }
    return xShapes;
}

const uno::Reference< container::XIndexContainer >&
    SvxMSConvertOCXControls::GetFormComps()
{
    if( !xFormComps.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {
            uno::Reference< form::XFormsSupplier > xFormsSupplier( xDrawPage,
                uno::UNO_QUERY );
            DBG_ASSERT( xFormsSupplier.is(),
                    "XFormsSupplier nicht vom XDrawPage erhalten" );

            uno::Reference< container::XNameContainer >  xNameCont =
                xFormsSupplier->getForms();

            // Das Formular bekommt einen Namen wie "WW-Standard[n]" und
            // wird in jedem Fall neu angelegt.
            UniString sName( sWW8_form, RTL_TEXTENCODING_ASCII_US );
            sal_uInt16 n = 0;

            while( xNameCont->hasByName( sName ) )
            {
                sName.AssignAscii( sWW8_form );
                sName += String::CreateFromInt32( ++n );
            }

            const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory
                = GetServiceFactory();
            if( !rServiceFactory.is() )
                return xFormComps;

            uno::Reference< uno::XInterface >  xCreate =
                rServiceFactory->createInstance(WW8_ASCII2STR(
                    "com.sun.star.form.component.Form"));
            if( xCreate.is() )
            {
                uno::Reference< beans::XPropertySet > xFormPropSet( xCreate,
                    uno::UNO_QUERY );

                uno::Any aTmp(&sName,getCppuType((OUString *)0));
                xFormPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

                uno::Reference< form::XForm > xForm( xCreate, uno::UNO_QUERY );
                DBG_ASSERT(xForm.is(), "keine Form?")

                uno::Reference< container::XIndexContainer > xForms( xNameCont,
                    uno::UNO_QUERY );
                DBG_ASSERT( xForms.is(), "XForms nicht erhalten" );

                aTmp.setValue( &xForm,
                    ::getCppuType((uno::Reference < form::XForm >*)0));
                xForms->insertByIndex( xForms->getCount(), aTmp );

                xFormComps = uno::Reference< container::XIndexContainer >
                    (xCreate, uno::UNO_QUERY);
            }
        }
    }

    return xFormComps;
}

sal_Bool OCX_CommandButton::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent >  &rFComp,
    awt::Size &rSz)
{
    OUString sServiceName =
        WW8_ASCII2STR("com.sun.star.form.component.CommandButton");
    uno::Reference< uno::XInterface > xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);

    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    if (pCaption)
    {
        UniString sTmp(pCaption,RTL_TEXTENCODING_ASCII_US);
        OUString sStr = sTmp;
        aTmp.setValue(&sStr,getCppuType((OUString *)0));
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_GroupBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    return bRet;
}

sal_Bool OCX_GroupBox::WriteContents(SvStorageStreamRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    return bRet;
}

sal_Bool OCX_CommandButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    rContents->SeekRel(8);

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Label"));
    OUString *pStr = (OUString *)aTmp.getValue();

    nCaptionLen = pStr->getLength();
    if (nCaptionLen)
    {
        nCaptionLen |= 0x80000000;
        *rContents << nCaptionLen;
        Align(rContents,4,TRUE);
        String aTmpStr(*pStr);
        ByteString sByte(aTmpStr,RTL_TEXTENCODING_ASCII_US);
        rContents->Write(sByte.GetBuffer(),sByte.Len());
    }

    Align(rContents,4,TRUE);

    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());

    bRet = aFontData.Export(rContents,rPropSet);

    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    sal_uInt8 nTmp = 0x27;
    if (nCaptionLen)
        nTmp |= 0x08;
    *rContents << nTmp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    DBG_ASSERT((rContents.Is() && (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_CommandButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x32, 0x05, 0xD7,
            0x69, 0xCE, 0xCD, 0x11, 0xA7, 0x77, 0x00, 0xDD,
            0x01, 0x14, 0x3C, 0x57, 0x22, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x46, 0x6F, 0x72, 0x6d, 0x73, 0x20,
            0x32, 0x2e, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x6D,
            0x61, 0x6E, 0x64, 0x42, 0x75, 0x74, 0x74, 0x6F,
            0x6E, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
            0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
            0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x16, 0x00,
            0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
            0x43, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x42,
            0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00,
            0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x6D, 0x00,
        0x61, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x42, 0x00,
        0x75, 0x00, 0x74, 0x00, 0x74, 0x00, 0x6F, 0x00,
        0x6E, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));

    return WriteContents(xContents,rPropSet,rSize);
}

sal_Bool OCX_ImageButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    rContents->SeekRel(8);

    uno::Any aTmp=rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    Align(rContents,4,TRUE);

    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());

    bRet = aFontData.Export(rContents,rPropSet);

    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    sal_uInt8 nTmp = 0x26;
    *rContents << nTmp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    DBG_ASSERT((rContents.Is() && (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_ImageButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x32, 0x05, 0xD7,
            0x69, 0xCE, 0xCD, 0x11, 0xA7, 0x77, 0x00, 0xDD,
            0x01, 0x14, 0x3C, 0x57, 0x22, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x46, 0x6F, 0x72, 0x6d, 0x73, 0x20,
            0x32, 0x2e, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x6D,
            0x61, 0x6E, 0x64, 0x42, 0x75, 0x74, 0x74, 0x6F,
            0x6E, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
            0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
            0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x16, 0x00,
            0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
            0x43, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x42,
            0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00,
            0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x6D, 0x00,
        0x61, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x42, 0x00,
        0x75, 0x00, 0x74, 0x00, 0x74, 0x00, 0x6F, 0x00,
        0x6E, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents,rPropSet,rSize);
}


sal_Bool OCX_OptionButton::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{
    OUString sServiceName =
        WW8_ASCII2STR("com.sun.star.form.component.RadioButton");
    uno::Reference< uno::XInterface >  xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet(xCreate,uno::UNO_QUERY);

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    if (pValue)
    {
        INT16 nTmp = pValue[0]-0x30;
        aTmp <<= nTmp;
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultState"), aTmp);
    }

    if (pCaption)
    {
        UniString sTmp(pCaption,RTL_TEXTENCODING_ASCII_US);
        OUString sStr = sTmp;
        aTmp.setValue(&sStr,getCppuType((OUString *)0));
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_OptionButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//=fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);


    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 5;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    Align(rContents,4,TRUE);
    nValueLen = 1|0x80000000;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultState"));
    INT16 nDefault;
    aTmp >>= nDefault;
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;


    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Label"));
    OUString *pStr = (OUString *)aTmp.getValue();
    nCaptionLen = pStr->getLength();
    ByteString sByte;
    if (nCaptionLen)
    {
        Align(rContents,4,TRUE);
        nCaptionLen |= 0x80000000;
        *rContents << nCaptionLen;
        pBlockFlags[2] |= 0x80;
        sByte = ByteString(String(*pStr),RTL_TEXTENCODING_ASCII_US);
    }

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nDefault += 0x30;
    *rContents << sal_uInt8(nDefault);
    *rContents << sal_uInt8(0x00);
    Align(rContents,4,TRUE);
    if (nCaptionLen)
        rContents->Write(sByte.GetBuffer(),sByte.Len());

    Align(rContents,4,TRUE);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());
    bRet = aFontData.Export(rContents,rPropSet);

    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_OptionButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x50, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x21, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4F, 0x70, 0x74, 0x69,
        0x6F, 0x6E, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E,
        0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D, 0x62,
        0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F, 0x62,
        0x6A, 0x65, 0x63, 0x74, 0x00, 0x15, 0x00, 0x00,
        0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E, 0x4F,
        0x70, 0x74, 0x69, 0x6F, 0x6E, 0x42, 0x75, 0x74,
        0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
            0x4F, 0x00, 0x70, 0x00, 0x74, 0x00, 0x69, 0x00,
            0x6F, 0x00, 0x6E, 0x00, 0x42, 0x00, 0x75, 0x00,
            0x74, 0x00, 0x74, 0x00, 0x6F, 0x00, 0x6E, 0x00,
            0x31, 0x00, 0x00, 0x00, 0x00, 0x00
            };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}


sal_Bool OCX_TextBox::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent >  &rFComp, awt::Size &rSz)
{
    OUString sServiceName =
        WW8_ASCII2STR("com.sun.star.form.component.TextField");
    uno::Reference< uno::XInterface >  xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if (fEnabled)
        bTemp = sal_True;
    else
        bTemp = sal_False;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    if (fLocked)
        bTemp = sal_True;
    else
        bTemp = sal_False;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);


    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    sal_Bool bTmp=fMultiLine;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    sal_uInt16 nTmp = static_cast<sal_uInt16>(nMaxLength);
    aTmp <<= nTmp;
    xPropSet->setPropertyValue( WW8_ASCII2STR("MaxTextLen"), aTmp);


    sal_Bool bTemp1,bTemp2;
    uno::Any aBarsH,aBarsV;
    switch(nScrollBars)
    {
        case 1:
            bTemp1 = sal_True;
            bTemp2 = sal_False;
            break;
        case 2:
            bTemp1 = sal_False;
            bTemp2 = sal_True;
            break;
        case 3:
            bTemp1 = sal_True;
            bTemp2 = sal_True;
            break;
        case 0:
        default:
            bTemp1 = sal_False;
            bTemp2 = sal_False;
            break;
    }

    aBarsH = bool2any(bTemp1);
    aBarsV = bool2any(bTemp2);
    xPropSet->setPropertyValue( WW8_ASCII2STR("HScroll"), aBarsH);
    xPropSet->setPropertyValue( WW8_ASCII2STR("VScroll"), aBarsV);

    nTmp = nPasswordChar;
    aTmp <<= nTmp;
    xPropSet->setPropertyValue( WW8_ASCII2STR("EchoChar"), aTmp);


    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);


    if (pValue)
    {
        UniString sTmp(pValue,RTL_TEXTENCODING_ASCII_US);
        OUString sStr = sTmp;
        aTmp.setValue(&sStr,getCppuType((OUString *)0));
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_TextBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x00;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;


    sal_uInt8 nTemp=0x19;
    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine"));
    fMultiLine = any2bool(aTmp);
    nTemp = 0x2C;
    if (fMultiLine)
        nTemp |= 0x80;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue( WW8_ASCII2STR("MaxTextLen"));
    aTmp >>= nMaxLength;
    *rContents << nMaxLength;
    pBlockFlags[0] |= 0x08;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    aTmp = rPropSet->getPropertyValue( WW8_ASCII2STR("HScroll"));
    sal_Bool bTemp1 = any2bool(aTmp);
    aTmp = rPropSet->getPropertyValue( WW8_ASCII2STR("VScroll"));
    sal_Bool bTemp2 = any2bool(aTmp);
    if (!bTemp1 && !bTemp2)
        nScrollBars =0;
    else if (bTemp1 && bTemp2)
        nScrollBars = 3;
    else if (!bTemp1 && bTemp2)
        nScrollBars = 2;
    else
        nScrollBars = 1;
    *rContents << nScrollBars;
    pBlockFlags[0] |= 0x20;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("EchoChar"));
    sal_uInt16 nTmp;
    aTmp >>= nTmp;
    nPasswordChar = static_cast<sal_uInt8>(nTmp);
    *rContents << nPasswordChar;
    pBlockFlags[1] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultText"));
    OUString *pStr = (OUString *)aTmp.getValue();
    nValueLen = pStr->getLength();
    ByteString sByte;
    if (nValueLen)
    {
        Align(rContents,4,TRUE);
        nValueLen |= 0x80000000;
        *rContents << nValueLen;
        pBlockFlags[2] |= 0x40;
        sByte = ByteString(String(*pStr),RTL_TEXTENCODING_ASCII_US);
    }

    Align(rContents,4,TRUE);
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    if (nValueLen)
        rContents->Write(sByte.GetBuffer(),sByte.Len());

    Align(rContents,4,TRUE);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());

    bRet = aFontData.Export(rContents,rPropSet);

    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK == rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_TextBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x54, 0x65, 0x78, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x54, 0x65, 0x78, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x54, 0x00, 0x65, 0x00, 0x78, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_FieldControl::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x00;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;


    sal_uInt8 nTemp=0x19;
    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    nTemp = 0x2C;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

#if 0 //Each control has a different Value format, and how to convert each to text has to be found out
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultText"));
    OUString *pStr = (OUString *)aTmp.getValue();
    nValueLen = pStr->getLength();
    ByteString sByte;
    if (nValueLen)
    {
        Align(rContents,4,TRUE);
        nValueLen |= 0x80000000;
        *rContents << nValueLen;
        pBlockFlags[2] |= 0x40;
        sByte = ByteString(String(*pStr),RTL_TEXTENCODING_ASCII_US);
    }
#endif

    Align(rContents,4,TRUE);
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

#if 0
    if (nValueLen)
        rContents->Write(sByte.GetBuffer(),sByte.Len());
#endif

    Align(rContents,4,TRUE);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());

    bRet = aFontData.Export(rContents,rPropSet);

    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}

sal_Bool OCX_FieldControl::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x54, 0x65, 0x78, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x54, 0x65, 0x78, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x54, 0x00, 0x65, 0x00, 0x78, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}



sal_Bool OCX_ToggleButton::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{

    OUString sServiceName =
        WW8_ASCII2STR("com.sun.star.form.component.CheckBox");
    uno::Reference< uno::XInterface >  xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet(xCreate,uno::UNO_QUERY);

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    if (pValue)
    {
        INT16 nTmp=pValue[0]-0x30;
        aTmp <<= nTmp;
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultState"), aTmp);
    }

    if (pCaption)
    {
        UniString sTmp(pCaption,RTL_TEXTENCODING_ASCII_US);
        OUString sStr = sTmp;
        aTmp.setValue(&sStr,getCppuType((OUString *)0));
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_Label::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent >  &rFComp,awt::Size &rSz)
{
    OUString sServiceName = WW8_ASCII2STR(
        "com.sun.star.form.component.FixedText");
    uno::Reference< uno::XInterface >  xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet(xCreate,uno::UNO_QUERY);

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    bTemp=fWordWrap;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    if (pCaption)
    {
        UniString sTmp(pCaption,RTL_TEXTENCODING_ASCII_US);
        OUString sStr = sTmp;
        aTmp.setValue(&sStr,getCppuType((OUString *)0));
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_ComboBox::Import(
    const uno::Reference< lang::XMultiServiceFactory >  &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{

    OUString sServiceName = WW8_ASCII2STR(
        "com.sun.star.form.component.ComboBox");
    uno::Reference< uno::XInterface > xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTmp=fEnabled;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTmp=fLocked;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);

    if (nDropButtonStyle)
        bTmp=sal_True;
    else
        bTmp=sal_False;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Dropdown"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    if (pValue)
        {
            UniString sTmp(pValue,RTL_TEXTENCODING_ASCII_US);
            OUString sStr = sTmp;
            aTmp.setValue(&sStr,getCppuType((OUString *)0));
            xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
        }

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    sal_Int16 nTmp=static_cast<sal_Int16>(nMaxLength);
    aTmp <<= nTmp;
    xPropSet->setPropertyValue( WW8_ASCII2STR("MaxTextLen"), aTmp);

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_ComboBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x00;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;


    sal_uInt8 nTemp=0x19;//fEnabled;
    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);
    *rContents << sal_uInt8(0x2C);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    nStyle = 3;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    Align(rContents,2,TRUE);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("LineCount"));
    aTmp >>= nListRows;
    *rContents << nListRows;
    pBlockFlags[1] |= 0x40;

    *rContents << sal_uInt8(1); //DefaultSelected One
    pBlockFlags[2] |= 0x01;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Dropdown"));
    nDropButtonStyle = any2bool(aTmp);
    if (nDropButtonStyle)
        nDropButtonStyle=0x02;
    *rContents << nDropButtonStyle;
    pBlockFlags[2] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Text"));
    OUString *pStr = (OUString *)aTmp.getValue();
    nValueLen = pStr->getLength();
    ByteString sByte;
    if (nValueLen)
    {
        Align(rContents,4,TRUE);
        sByte = ByteString(String(*pStr),RTL_TEXTENCODING_ASCII_US);
        nValueLen |= 0x80000000;
        *rContents << nValueLen;
        pBlockFlags[2] |= 0x40;
    }

    Align(rContents,4,TRUE);
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    if (nValueLen)
        rContents->Write(sByte.GetBuffer(),sByte.Len());

    Align(rContents,4,TRUE);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());

    bRet = aFontData.Export(rContents,rPropSet);

    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_ComboBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x30, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1D, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x62,
        0x6F, 0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00,
        0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65,
        0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74,
        0x00, 0x11, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72,
        0x6D, 0x73, 0x2E, 0x43, 0x6F, 0x6D, 0x62, 0x6F,
        0x42, 0x6F, 0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x62, 0x00,
        0x6F, 0x00, 0x42, 0x00, 0x6F, 0x00, 0x78, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}



sal_Bool OCX_ListBox::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{
    OUString sServiceName = WW8_ASCII2STR("com.sun.star.form.component.ListBox");
    uno::Reference< uno::XInterface > xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet >  xPropSet(xCreate,uno::UNO_QUERY);

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTmp=fEnabled;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTmp=fLocked;
    aTmp = bool2any(bTmp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    sal_Bool bTemp = nMultiState;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("MultiSelection"), aTmp);

#if 0       //Don't delete this for now until I figure out if I can make this
    if (pValue)
        {
            UniString sTmp(pValue,RTL_TEXTENCODING_ASCII_US);
            OUString sStr = sTmp;
            aTmp.setValue(&sStr,getCppuType((OUString *)0));
            xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
        }
#endif

    aTmp <<= ImportColor(nBackColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    aFontData.Import(xPropSet);
    return sal_True;
}

sal_Bool OCX_ListBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x01;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;



    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiSelection"));
    nMultiState = any2bool(aTmp);

    if (nMultiState)
    {
        *rContents << nMultiState;
        pBlockFlags[0] |= 0x20;
    }

    nStyle = 2;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;


    Align(rContents,4,TRUE);

#if 0
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultText"));
    OUString *pStr = (OUString *)aTmp.getValue();
    nValueLen = pStr->getLength();
    ByteString sByte;
    if (nValueLen)
    {
        sByte = ByteString(String(*pStr),RTL_TEXTENCODING_ASCII_US);
        nValueLen |= 0x80000000;
        *rContents << nValueLen;
        pBlockFlags[2] |= 0x40;
    }
    Align(rContents,4,TRUE);
#endif

    *rContents << rSize.Width;
    *rContents << rSize.Height;

#if 0
    if (nValueLen)
    {
        rContents->Write(sByte.GetBuffer(),sByte.Len());
        Align(rContents,4,TRUE);
    }
#endif

    Align(rContents,4,TRUE);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());

    bRet = aFontData.Export(rContents,rPropSet);

    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}

sal_Bool OCX_ListBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4C, 0x69, 0x73, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x4C, 0x69, 0x73, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x4C, 0x00, 0x69, 0x00, 0x73, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_ModernControl::Read(SvStorageStream *pS)
{
    *pS >> nIdentifier;
    DBG_ASSERT(nIdentifier==nStandardId,
            "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,8);

    if (pBlockFlags[0] & 0x01)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;

        fEnabled = (nTemp & 0x02) >> 1;
        fLocked = (nTemp & 0x04) >> 2;
        fBackStyle = (nTemp & 0x02) >> 3;

        *pS >> nTemp;

        fColumnHeads = (nTemp & 0x04) >> 2;
        fIntegralHeight = (nTemp & 0x08) >> 3;
        fMatchRequired = (nTemp & 0x10) >> 4;
        fAlignment = (nTemp & 0x20) >> 5;

        *pS >> nTemp;

        fDragBehaviour = (nTemp & 0x08) >> 3;
        fEnterKeyBehaviour = (nTemp & 0x10) >> 4;
        fEnterFieldBehaviour = (nTemp & 0x20) >> 5;
        fTabKeyBehaviour = (nTemp & 0x40) >> 6;
        fWordWrap = (nTemp & 0x80) >> 7;

        *pS >> nTemp;
        fSelectionMargin = (nTemp & 0x04) >> 2;
        fAutoWordSelect = (nTemp & 0x08) >> 3;
        fAutoSize = (nTemp & 0x10) >> 4;
        fHideSelection = (nTemp & 0x20) >> 5;
        fAutoTab = (nTemp & 0x40) >> 6;
        fMultiLine = (nTemp & 0x80) >> 7;

    }

    /*If any of these are set they follow eachother in this order one after
    another padded out to the next U32 boundary with 0's
    U8 can abut each other U16 must start on a U16 boundary and are padded to
    that with 0's. A standardish word alignment structure*/

    if (pBlockFlags[0] & 0x02)
        *pS >> nBackColor;
    if (pBlockFlags[0] & 0x04)
        *pS >> nForeColor;
    if (pBlockFlags[0] & 0x08)
        *pS >> nMaxLength;

    if (pBlockFlags[0] & 0x10)
        *pS >> nBorderStyle;
    if (pBlockFlags[0] & 0x20)
        *pS >> nScrollBars;
    if (pBlockFlags[0] & 0x40)
        *pS >> nStyle;// (UI 0 == Data 3, UI 2 = Data 7)
    if (pBlockFlags[0] & 0x80)
        *pS >> nMousePointer;

    if (pBlockFlags[1] & 0x02)
        *pS >> nPasswordChar; //HUH ??? always 0 ??? not sure maybe just padding

    if (pBlockFlags[1] & 0x04)
    {
        Align(pS,4);
        *pS >> nListWidth;
    }

    if (pBlockFlags[1] & 0x08)
    {
        Align(pS,2);
        *pS >> nBoundColumn;
    }
    if (pBlockFlags[1] & 0x10)
    {
        Align(pS,2);
        *pS >> nTextColumn;
    }
    if (pBlockFlags[1] & 0x20)
    {
        Align(pS,2);
        *pS >> nColumnCount;
    }
    if (pBlockFlags[1] & 0x40)
    {
        Align(pS,2);
        *pS >> nListRows;
    }
    if (pBlockFlags[1] & 0x80)
    {
        Align(pS,2);
        *pS >> nUnknown8; //something to do with ColumnWidths
    }
    if (pBlockFlags[2] & 0x01)
        *pS >> nMatchEntry;
    if (pBlockFlags[2] & 0x02)
        *pS >> nListStyle;
    if (pBlockFlags[2] & 0x04)
        *pS >> nShowDropButtonWhen;
    if (pBlockFlags[2] & 0x10)
        *pS >> nDropButtonStyle;
    if (pBlockFlags[2] & 0x20)
        *pS >> nMultiState;

    if (pBlockFlags[2] & 0x40)
    {
        Align(pS,4);
        *pS >> nValueLen; //bit 0x80000000L set
    }
    if (pBlockFlags[2] & 0x80)
    {
        Align(pS,4);
        *pS >> nCaptionLen; //bit 0x80000000L set
    }
    if (pBlockFlags[3] & 0x01)
    {
        Align(pS,4);
        *pS >> nHorzPos;
        *pS >> nVertPos;
    }
    if (pBlockFlags[3] & 0x02)
    {
        Align(pS,4);
        *pS >> nBorderColor;
    }
    if (pBlockFlags[3] & 0x04)
    {
        Align(pS,4); // NEW
        *pS >> nSpecialEffect;
    }
    if (pBlockFlags[3] & 0x08)
    {
        Align(pS,2);
        *pS >> nIcon;
        DBG_ASSERT(nIcon == 0xFFFF, "Unexpected nIcon");
    }
    if (pBlockFlags[3] & 0x10)
    {
        Align(pS,2);
        *pS >> nPicture;
        DBG_ASSERT(nPicture == 0xFFFF, "Unexpected nIcon");
    }
    if (pBlockFlags[3] & 0x20)
        *pS >> nAccelerator;
    /*
    if (pBlockFlags[3] & 0x80)
        *pS >> nUnknown9;
    */
    if (pBlockFlags[4] & 0x01)
    {
        Align(pS,4);
        *pS >> nGroupNameLen; // bit 0x80000000L set
    }

    //End

    Align(pS,4);
    *pS >> nWidth;
    *pS >> nHeight;

    if (nValueLen)
    {
        nValueLen &= 0x7FFFFFFF;
        pValue= new char[nValueLen+1];
        pS->Read(pValue,nValueLen);
        pValue[nValueLen]=0;
        Align(pS,4);
    }

    if (nCaptionLen)
    {
        nCaptionLen &= 0x7FFFFFFF;
        pCaption = new char[nCaptionLen+1];
        pS->Read(pCaption,nCaptionLen);
        pCaption[nCaptionLen]=0;
        Align(pS,4);
    }

    if (nGroupNameLen)
    {
        nGroupNameLen &= 0x7FFFFFFF;
        pGroupName = new char[nGroupNameLen+1];
        pS->Read(pGroupName,nGroupNameLen);
        pGroupName[nGroupNameLen]=0;
        Align(pS,4);
    }

    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        pPicture = new sal_uInt8[nPictureLen];
        pS->Read(pPicture,nPictureLen);
    }

    return sal_True;
}


sal_Bool OCX_CommandButton::Read(SvStorageStream *pS)
{
    *pS >> nIdentifier;
    DBG_ASSERT(nStandardId==nIdentifier,
        "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);


    if (pBlockFlags[0] & 0x01)
        *pS >> nForeColor;
    if (pBlockFlags[0] & 0x02)
        *pS >> nBackColor;

    if (pBlockFlags[0] & 0x04)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;
        fEnabled = (nTemp&0x02)>>1;
        fLocked = (nTemp&0x04)>>2;
        fBackStyle = (nTemp&0x08)>>3;
        *pS >> nTemp;
        *pS >> nTemp;
        fWordWrap = (nTemp&0x80)>>7;
        *pS >> nTemp;
        fAutoSize = (nTemp&0x10)>>4;
    }

    if (pBlockFlags[0] & 0x08)
        *pS >> nCaptionLen;

    if (pBlockFlags[0] & 0x10) /*Picture Position, a strange mechanism here*/
    {
        *pS >> nVertPos;
        *pS >> nHorzPos;
    }

    if (pBlockFlags[0] & 0x40) /*MousePointer*/
        *pS >> nMousePointer;

    if (pBlockFlags[0] & 0x80)
    {
        Align(pS,2);
        *pS >> nPicture;
    }

    if (pBlockFlags[1] & 0x01)
    {
        Align(pS,2);
        *pS >> nAccelerator;
    }

    if (pBlockFlags[1] & 0x04)
    {
        Align(pS,2);
        *pS >> nIcon;
    }

    if (nCaptionLen)
    {
        Align(pS,4);
        nCaptionLen &= 0x7FFFFFFF;
        pCaption = new char[nCaptionLen+1];
        pS->Read(pCaption,nCaptionLen);
        pCaption[nCaptionLen]=0;
    }

    Align(pS,4);
    *pS >> nWidth;
    *pS >> nHeight;

    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        pPicture = new sal_uInt8[nPictureLen];
        pS->Read(pPicture,nPictureLen);
    }

    return sal_True;
}

sal_Bool OCX_Label::Read(SvStorageStream *pS)
{
    *pS >> nIdentifier;
    DBG_ASSERT(nStandardId==nIdentifier,
            "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);


    if (pBlockFlags[0] & 0x01)
        *pS >> nForeColor;
    if (pBlockFlags[0] & 0x02)
        *pS >> nBackColor;


    if (pBlockFlags[0] & 0x04)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;
        fEnabled = (nTemp&0x02)>>1;
        fLocked = (nTemp&0x04)>>2;
        fBackStyle = (nTemp&0x08)>>3;
        *pS >> nTemp;
        *pS >> nTemp;
        fWordWrap = (nTemp&0x80)>>7;
        *pS >> nTemp;
        fAutoSize = (nTemp&0x10)>>4;
    }
    if (pBlockFlags[0] & 0x08)
        *pS >> nCaptionLen;

    if (pBlockFlags[0] & 0x10)
    {
        *pS >> nVertPos;
        *pS >> nHorzPos;
    }

    if (pBlockFlags[0] & 0x40)
        *pS >> nMousePointer;

    if (pBlockFlags[0] & 0x80)
    {
        Align(pS,4);
        *pS >> nBorderColor;
    }

    if (pBlockFlags[1] & 0x01)
    {
        Align(pS,2);
        *pS >> nBorderStyle;
    }

    if (pBlockFlags[1] & 0x02)
    {
        Align(pS,2);
        *pS >> nSpecialEffect;
    }

    if (pBlockFlags[1] & 0x04)
    {
        Align(pS,2);
        *pS >> nPicture;
    }

    if (pBlockFlags[1] & 0x08)
    {
        Align(pS,2);
        *pS >> nAccelerator;
    }

    if (pBlockFlags[1] & 0x10)
    {
        Align(pS,2);
        *pS >> nIcon;
    }

    if (nCaptionLen)
    {
        Align(pS,4);
        nCaptionLen &= 0x7FFFFFFF;
        pCaption = new char[nCaptionLen+1];
        pS->Read(pCaption,nCaptionLen);
        pCaption[nCaptionLen]=0;
        Align(pS,4);
    }

    *pS >> nWidth;
    *pS >> nHeight;

    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        pPicture = new sal_uInt8[nPictureLen];
        pS->Read(pPicture,nPictureLen);
    }

    return sal_True;
}

sal_Bool OCX_Label::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet = sal_True;
    rContents->SeekRel(8);
    pBlockFlags[0] = 0x20;
    pBlockFlags[1] = 0;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x01;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine"));
    fWordWrap = any2bool(aTmp);
    nTemp=fWordWrap;
    nTemp = nTemp << 7;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Label"));
    OUString *pStr = (OUString *)aTmp.getValue();
    nCaptionLen = pStr->getLength();
    ByteString sByte;
    if (nCaptionLen)
    {
        nCaptionLen |= 0x80000000;
        *rContents << nCaptionLen;
        pBlockFlags[0] |= 0x08;
        sByte = ByteString(String(*pStr),RTL_TEXTENCODING_ASCII_US);
    }

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    sal_uInt8 nNewBorder;
    nSpecialEffect = ExportBorder(nBorder,nNewBorder);
    nBorderStyle = nNewBorder;
    *rContents << nBorderStyle;
    pBlockFlags[1] |= 0x01;
    *rContents << nSpecialEffect;
    pBlockFlags[1] |= 0x02;

    if (nCaptionLen)
        rContents->Write(sByte.GetBuffer(),sByte.Len());

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());

    bRet = aFontData.Export(rContents,rPropSet);

    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_Label::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x23, 0x9E, 0x8C, 0x97,
        0xB0, 0xD4, 0xCE, 0x11, 0xBF, 0x2D, 0x00, 0xAA,
        0x00, 0x3F, 0x40, 0xD0, 0x1A, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4C, 0x61, 0x62, 0x65,
        0x6C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
        0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
        0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x0E, 0x00,
        0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
        0x4C, 0x61, 0x62, 0x65, 0x6C, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x4C, 0x00, 0x61, 0x00, 0x62, 0x00, 0x65, 0x00,
        0x6C, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aTest[] = {
        0x00, 0x02, 0x20, 0x00, 0x2B, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
        0x06, 0x00, 0x00, 0x80, 0x4C, 0x61, 0x62, 0x65,
        0x6C, 0x31, 0x18, 0x00, 0xEC, 0x09, 0x00, 0x00,
        0x7B, 0x02, 0x00, 0x00, 0x00, 0x02, 0x20, 0x00,
        0x35, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x80,
        0xC3, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
        0x54, 0x69, 0x6D, 0x65, 0x73, 0x20, 0x4E, 0x65,
        0x77, 0x20, 0x52, 0x6F, 0x6D, 0x61, 0x6E, 0x00,
        };

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

typedef OCX_Control *(*FNCreate)();


struct OCX_map
{
    FNCreate pCreate;
    const char *sId;
    sal_Int16 nId;
    const char *sName;
};

static const int NO_OCX=16;

OCX_map aOCXTab[NO_OCX] =
{
    {&OCX_CommandButton::Create,"D7053240-CE69-11CD-a777-00dd01143c57",
        form::FormComponentType::COMMANDBUTTON,"CommandButton"},
    {&OCX_Label::Create, "978C9E23-D4B0-11CE-bf2d-00aa003f40d0",
        form::FormComponentType::FIXEDTEXT,"Label"},
    {&OCX_TextBox::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::TEXTFIELD,"TextBox"},
    {&OCX_ListBox::Create,"8BD21D20-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::LISTBOX,"ListBox"},
    {&OCX_ComboBox::Create,"8BD21D30-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::COMBOBOX,"ComboBox"},
    {&OCX_CheckBox::Create,"8BD21D40-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::CHECKBOX,"CheckBox"},
    {&OCX_OptionButton::Create,"8BD21D50-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::RADIOBUTTON,"OptionButton"},
    {&OCX_ToggleButton::Create,"8BD21D60-EC42-11CE-9e0d-00aa006002f3",
        -1,"ToggleButton"},
    {&OCX_Image::Create,"4C599241-6926-101B-9992-00000b65c6f9",
        form::FormComponentType::IMAGECONTROL,"Image"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::DATEFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::TIMEFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::NUMERICFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::CURRENCYFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::PATTERNFIELD,"TextBox"},
#if 0
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::FORMULAFIELD,"TextBox"},
#endif
    {&OCX_ImageButton::Create,"D7053240-CE69-11CD-a777-00dd01143c57",
        form::FormComponentType::IMAGEBUTTON,"CommandButton"},
    {&OCX_GroupBox::Create,"",
        form::FormComponentType::GROUPBOX,""}
};

OCX_Control * SvxMSConvertOCXControls::OCX_Factory(const String &sName)
{
    for (int i=0;i<NO_OCX;i++)
    {
        if ( sName.EqualsAscii( aOCXTab[i].sId ))
            return(aOCXTab[i].pCreate());
    }
    return(NULL);
}

OCX_Control * SvxMSConvertOCXControls::OCX_Factory(
    const uno::Reference< awt::XControlModel > &rControlModel,
    String &rId, String &rName)
{
    rName.Erase();
    rId.Erase();

    uno::Reference< beans::XPropertySet > xPropSet(
        rControlModel,uno::UNO_QUERY);

    uno::Any aTmp = xPropSet->getPropertyValue(
        OUString::createFromAscii("ClassId"));
    sal_Int16 nClassId = *(sal_Int16*) aTmp.getValue();

//Begin nasty hack
    /*
    There is a truly horrible thing with EditControls and FormattedField
    Controls, they both pretend to have an EDITBOX ClassId for compability
    reasons, at some stage in the future hopefully there will be a proper
    FormulaField ClassId rather than this piggybacking two controls onto the
    same ClassId, when this happens uncomment the FORMULAFIELD in the OCX_Tab
    and delete this block, cmc.

    And also the nClassId for ImageControls is being reported as "CONTROL"
    rather than IMAGECONTROL
    */
    if (nClassId == form::FormComponentType::TEXTFIELD)
    {
        uno::Reference< lang::XServiceInfo > xInfo(rControlModel,
            uno::UNO_QUERY);
        if (xInfo->
            supportsService(OUString::createFromAscii(
                "com.sun.star.form.component.FormattedField")))
        {
            rId.AppendAscii("8BD21D10-EC42-11CE-9e0d-00aa006002f3");
            rName.AppendAscii("TextBox");
            return new OCX_FieldControl;
        }
    }
    else if (nClassId == form::FormComponentType::CONTROL)
    {
        uno::Reference< lang::XServiceInfo > xInfo(rControlModel,
            uno::UNO_QUERY);
        if (xInfo->
            supportsService(OUString::createFromAscii(
                "com.sun.star.form.component.ImageControl")))
        nClassId = form::FormComponentType::IMAGECONTROL;
    }
//End nasty hack

    //sal_Int16  nClassId = FormComponentType::COMMANDBUTTON;

    for (int i=0;i<NO_OCX;i++)
    {
        if ( nClassId == aOCXTab[i].nId )
        {
            rId.AppendAscii(aOCXTab[i].sId);
            rName.AppendAscii(aOCXTab[i].sName);
            return(aOCXTab[i].pCreate());
        }
    }
    return(NULL);
}


sal_Bool SvxMSConvertOCXControls::ReadOCXStream( SvStorageRef& rSrc1,
        uno::Reference < drawing::XShape > *pShapeRef,BOOL bFloatingCtrl)
{
    sal_Bool bRet=sal_False;

    SvStorageStreamRef xSrc2 = rSrc1->OpenStream( WW8_ASCII2STR("\3OCXNAME") );
    SvStorageStream* pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    xSrc2 = rSrc1->OpenStream( WW8_ASCII2STR("contents") );
    pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    /*Get Class Id of this object, see if it is one of the types
     *that this importer can handle, call the appropiate handler
     to read that control, and call the appropiate handler to
     insert that control
     */

    OCX_Control *pObj=NULL;
    SvGlobalName aTest = rSrc1->GetClassName();
    if (pObj = OCX_Factory(aTest.GetHexName()))
    {

        com::sun::star::awt::Size aSz;
        uno::Reference< form::XFormComponent >  xFComp;
        const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
            GetServiceFactory();
        if(!rServiceFactory.is())
            return(sal_False);
        if(bRet = pObj->FullRead(pSt))
            if (pObj->Import(rServiceFactory,xFComp,aSz))
                bRet = InsertControl( xFComp, aSz,pShapeRef,bFloatingCtrl);
        delete pObj;
    }
    return bRet;
}


sal_Bool SvxMSConvertOCXControls::ReadOCXExcelKludgeStream(
    SvStorageStreamRef& rSrc1, uno::Reference < drawing::XShape > *
    pShapeRef,BOOL bFloatingCtrl)
{
    sal_Bool bRet=sal_False;
    /*Get Class Id of this object, see if it is one of the types
     *that this importer can handle, call the appropiate handler
     to read that control, and call the appropiate handler to
     insert that control
     */
    /*The Excel Kludge is to concatenate a class id with a contents
     * stream, and then concatenate all the controls together,
     * This means that you should have the cnts stream wound to the
     * correct location before passing the control stream in here*/
    OCX_Control *pObj=NULL;
    SvStream *pSt = rSrc1;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    SvGlobalName aTest;
    *pSt >> aTest;
    if (pObj = OCX_Factory(aTest.GetHexName()))
    {

        com::sun::star::awt::Size aSz;
        uno::Reference< form::XFormComponent >  xFComp;
        const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
            GetServiceFactory();
        if(!rServiceFactory.is())
            return(sal_False);
        if(bRet = pObj->FullRead(rSrc1))
            if (pObj->Import(rServiceFactory,xFComp,aSz))
                bRet = InsertControl( xFComp, aSz,pShapeRef,bFloatingCtrl);
        delete pObj;
    }
    return bRet;
}


sal_Bool SvxMSConvertOCXControls::WriteOCXStream( SvStorageRef& rSrc1,
    const uno::Reference< awt::XControlModel > &rControlModel,
    const awt::Size &rSize, String &rName)
{
    sal_Bool bRet=sal_False;

    DBG_ASSERT( rControlModel.is(), "UNO-Control missing Model, panic!" );
    if( !rControlModel.is() )
        return sal_False;

#if 0
    uno::Any aTmp = xPropSet->getPropertyValue(
        OUString::createFromAscii("ClassId"));
    sal_Int16 nClassId = *(sal_Int16*) aTmp.getValue();
#endif

    OCX_Control *pObj=NULL;
    String sId;
    if (pObj = OCX_Factory(rControlModel,sId,rName))
    {
        uno::Reference<beans::XPropertySet> xPropSet(rControlModel,
            uno::UNO_QUERY);

        SvGlobalName aName;
        aName.MakeId(sId);
        String sFullName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
            "Microsoft Forms 2.0 ")));
        sFullName.Append(rName);
        rSrc1->SetClass(aName,0x5C,sFullName);
        //                    ^^^^, this pathetic 0x5c is the magic number
        // which the lack of originally blocked the implementation of
        // the controls export
        // cmc

        bRet = pObj->Export(rSrc1,xPropSet,rSize);
        delete pObj;
    }
    return bRet;
}


//I think this should work for excel documents, create the Cnts stream
//and give it here as rContents, we'll append out streams ole id and
//contents here and that appears to be what Excel is doing
sal_Bool SvxMSConvertOCXControls::WriteOCXExcelKludgeStream(
    SvStorageStreamRef& rContents,
    const uno::Reference< awt::XControlModel > &rControlModel,
    const awt::Size &rSize, String &rName)
{
    sal_Bool bRet=sal_False;

    DBG_ASSERT( rControlModel.is(), "UNO-Control missing Model, panic!" );
    if( !rControlModel.is() )
        return sal_False;

    OCX_Control *pObj=NULL;
    String sId;
    if (pObj = OCX_Factory(rControlModel,sId,rName))
    {
        uno::Reference<beans::XPropertySet> xPropSet(rControlModel,
            uno::UNO_QUERY);

        SvGlobalName aName;
        aName.MakeId(sId);
        String sFullName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
            "Microsoft Forms 2.0 ")));
        sFullName.Append(rName);
        SvStream *pS=rContents;
        *pS << aName;
        bRet = pObj->WriteContents(rContents,xPropSet,rSize);
        delete pObj;
    }
    return bRet;
}







sal_Bool OCX_CheckBox::Import(
    const uno::Reference< lang::XMultiServiceFactory >  &rServiceFactory,
    uno::Reference< form::XFormComponent > &rFComp, awt::Size &rSz)
{
    OUString sServiceName = WW8_ASCII2STR("com.sun.star.form.component.CheckBox");
    uno::Reference< uno::XInterface > xCreate =
        rServiceFactory->createInstance( sServiceName );
    if( !xCreate.is() )
        return(sal_False);

    rFComp = uno::Reference< form::XFormComponent > (xCreate,uno::UNO_QUERY);
    if( !rFComp.is() )
        return(sal_False);

    uno::Reference< beans::XPropertySet >  xPropSet( xCreate,uno::UNO_QUERY );

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= ImportColor(nForeColor);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    bTemp = nMultiState;
    aTmp = bool2any(bTemp);
    xPropSet->setPropertyValue( WW8_ASCII2STR("TriState"), aTmp);

    if (pValue)
    {
        INT16 nTmp=pValue[0]-0x30;
        aTmp <<= nTmp;
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultState"), aTmp);
    }

    if (pCaption)
    {
        UniString sTmp(pCaption,RTL_TEXTENCODING_ASCII_US);
        OUString sStr = sTmp;
        aTmp.setValue(&sStr,getCppuType((OUString *)0));
        xPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(xPropSet);
    return(sal_True);
}

sal_Bool OCX_CheckBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)

{
    sal_Bool bRet=sal_True;

    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    aTmp >>= nForeColor;
    *rContents << ExportColor(nForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 4;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TriState"));
    nMultiState = any2bool(aTmp);
    *rContents << nMultiState;
    pBlockFlags[2] |= 0x20;

    Align(rContents,4,TRUE);
    nValueLen = 1|0x80000000;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultState"));
    INT16 nDefault;
    aTmp >>= nDefault;
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Label"));
    OUString *pStr = (OUString *)aTmp.getValue();
    nCaptionLen = pStr->getLength();
    ByteString sByte;
    if (nCaptionLen)
    {
        Align(rContents,4,TRUE);
        nCaptionLen |= 0x80000000;
        *rContents << nCaptionLen;
        pBlockFlags[2] |= 0x80;
        sByte = ByteString(String(*pStr),RTL_TEXTENCODING_ASCII_US);
    }

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nDefault += 0x30;
    *rContents << sal_uInt8(nDefault);
    *rContents << sal_uInt8(0x00);
    Align(rContents,4,TRUE);
    if (nCaptionLen)
        rContents->Write(sByte.GetBuffer(),sByte.Len());

    Align(rContents,4,TRUE);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());
    bRet = aFontData.Export(rContents,rPropSet);
    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_CheckBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)

{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1D, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x43, 0x68, 0x65, 0x63,
        0x6B, 0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00,
        0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65,
        0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74,
        0x00, 0x11, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72,
        0x6D, 0x73, 0x2E, 0x43, 0x68, 0x65, 0x63, 0x6B,
        0x42, 0x6F, 0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x68, 0x00, 0x65, 0x00, 0x63, 0x00,
        0x6B, 0x00, 0x42, 0x00, 0x6F, 0x00, 0x78, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aTest[] = {
        0x00, 0x02, 0x34, 0x00, 0x46, 0x01, 0xC0, 0x80,
        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x80, 0x09, 0x00, 0x00, 0x80,
        0xE2, 0x0E, 0x00, 0x00, 0x95, 0x02, 0x00, 0x00,
        0x30, 0x69, 0x1D, 0x00, 0x43, 0x68, 0x65, 0x63,
        0x6B, 0x42, 0x6F, 0x78, 0x31, 0x20, 0x52, 0x6F,
        0x00, 0x02, 0x20, 0x00, 0x35, 0x00, 0x00, 0x00,
        0x0F, 0x00, 0x00, 0x80, 0xC3, 0x00, 0x00, 0x00,
        0x00, 0x02, 0x00, 0x00, 0x54, 0x69, 0x6D, 0x65,
        0x73, 0x20, 0x4E, 0x65, 0x77, 0x20, 0x52, 0x6F,
        0x6D, 0x61, 0x6E, 0x00,
    };
    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}


sal_Bool OCX_FontData::Read(SvStorageStream *pS)
{
    *pS >> nIdentifier;
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);

    if (pBlockFlags[0] & 0x01)
        *pS >> nFontNameLen;
    if (pBlockFlags[0] & 0x02)
    {
        sal_uInt8 nTmp;
        *pS >> nTmp;
        fBold = nTmp & 0x01;
        fItalic = (nTmp & 0x02) >> 1;
        fUnderline = (nTmp & 0x04) >> 2;
        fStrike = (nTmp & 0x08) >> 3;
        fUnknown1 = (nTmp & 0xF0) >> 4;
        *pS >> nUnknown2;
        *pS >> nUnknown3;
        *pS >> nUnknown4;
    }
    if (pBlockFlags[0] & 0x04)
    {
        Align(pS,4);
        *pS >> nFontSize;
    }
    if (pBlockFlags[0] & 0x10)
    {
        Align(pS,2);
        *pS >> nLanguageID;
    }
    if (pBlockFlags[0] & 0x40)
        *pS >> nJustification;

    if (nFontNameLen)
    {
        Align(pS,4);
        nFontNameLen &= 0x7FFFFFFF;
        pFontName = new char[nFontNameLen+1];
        pS->Read(pFontName,nFontNameLen);
        pFontName[nFontNameLen]=0;
    }

    Align(pS,4);
    return(TRUE);
}

void OCX_FontData::Import(uno::Reference< beans::XPropertySet > &rPropSet)
{
    uno::Any aTmp;
    if (pFontName)
    {
        UniString sTmp(pFontName,RTL_TEXTENCODING_ASCII_US);
        OUString sStr = sTmp;
        aTmp.setValue(&sStr,getCppuType((OUString *)0));
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontName"), aTmp);
    }

    if (bHasAlign)
    {
        sal_Int16 nAlign = ImportAlign(nJustification);
        aTmp <<= nAlign;
        rPropSet->setPropertyValue( WW8_ASCII2STR("Align"), aTmp);
    }

    if (fBold)
    {
        float nBold=150;
        aTmp.setValue(&nBold,getCppuType((float *)0));
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontWeight"), aTmp);
    }
}

sal_Bool OCX_FontData::Export(SvStorageStreamRef &rContent,
    const uno::Reference< beans::XPropertySet > &rPropSet)
{
    sal_uInt8 nFlags=0x00;
    nFixedAreaLen = static_cast<sal_uInt16>(rContent->Tell());
    rContent->SeekRel(8);
    ByteString sByte;
    uno::Any aTmp;

    if (bHasFont)
    {
        aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("FontName"));
        OUString *pStr = (OUString *)aTmp.getValue();
        nFontNameLen = pStr->getLength();
        if (nFontNameLen)
            sByte = ByteString(String(*pStr),RTL_TEXTENCODING_ASCII_US);
    }
    if (!nFontNameLen)
    {
        sByte = ByteString("Times New Roman");
        nFontNameLen = sByte.Len();
    }
    nFontNameLen |= 0x80000000;
    *rContent << nFontNameLen;
    nFlags |= 0x01;

    if (bHasFont)
    {
        aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("FontWeight"));
        float nBold;
        aTmp >>= nBold;

        if (nBold >= 150)
        {
            nFlags |= 0x02;
            sal_uInt8 nTmp=0x01;
            *rContent << nTmp;
            nTmp=0x00;
            *rContent << nTmp;
            *rContent << nTmp;
            *rContent << nTmp;
        }

        if (bHasAlign)
        {
            *rContent << sal_uInt16(0x0200);
            nFlags |= 0x10;

            nFlags |= 0x20; // ?

            aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Align"));
            nFlags |= 0x40;
            sal_Int16 nAlign;
            aTmp >>= nAlign;
            nJustification = ExportAlign(nAlign);
            *rContent << nJustification;
        }
    }

    Align(rContent,4,TRUE);
    rContent->Write(sByte.GetBuffer(),sByte.Len());
    Align(rContent,4,TRUE);

    UINT32 nOldPos = nFixedAreaLen;
    nFixedAreaLen = static_cast<sal_uInt16>(rContent->Tell()-nFixedAreaLen);
    nFixedAreaLen -= 4;
    rContent->Seek(nOldPos);
    *rContent << nStandardId;
    *rContent << nFixedAreaLen;
    *rContent << nFlags;
    *rContent << sal_uInt8(0x00);
    *rContent << sal_uInt8(0x00);
    *rContent << sal_uInt8(0x00);

    Align(rContent,4,TRUE);
    return sal_True;
}

sal_Bool OCX_Image::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    rContents->SeekRel(8);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x02;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(
        WW8_ASCII2STR("BackgroundColor"));
    aTmp >>= nBackColor;
    *rContents << ExportColor(nBackColor);
    pBlockFlags[0] |= 0x10;


    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder;
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x20;

    *rContents << nSpecialEffect;
    pBlockFlags[1] |= 0x01;

    Align(rContents,4,TRUE);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
    {
        sal_uInt8 nTemp = 0x19;
        *rContents << nTemp;
        pBlockFlags[1] |= 0x20;
        *rContents << sal_uInt8(0x00);
        *rContents << sal_uInt8(0x00);
        *rContents << sal_uInt8(0x00);
    }

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ImageURL"));
    OUString *pStr = (OUString *)aTmp.getValue();
    /*Magically fetch that image and turn it into something that
     *we can store in ms controls, wmf,png,jpg are almost certainly
     *the options we have for export...*/

    Align(rContents,4,TRUE);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    Align(rContents,4,TRUE);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell());

    nFixedAreaLen-=4;
    rContents->Seek(0);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_Image::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x92, 0x59, 0x4C,
        0x26, 0x69, 0x1B, 0x10, 0x99, 0x92, 0x00, 0x00,
        0x0B, 0x65, 0xC6, 0xF9, 0x1A, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x49, 0x6D, 0x61, 0x67,
        0x65, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
        0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
        0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x0E, 0x00,
        0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
        0x49, 0x6D, 0x61, 0x67, 0x65, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x49, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x67, 0x00,
        0x65, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}
