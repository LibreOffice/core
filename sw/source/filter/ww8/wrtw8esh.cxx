/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#include <com/sun/star/embed/Aspects.hpp>

#include <hintids.hxx>

#include <vcl/cvtgrf.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <vcl/svapp.hxx>
#include <sot/storage.hxx>
#include <svtools/filter.hxx>
#include <svl/itemiter.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <svx/unoshape.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/editeng.hxx>
#include <editeng/flditem.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svx/unoapi.hxx>

// #i71538#
#include <svx/svdview.hxx>
#include <fmtcnct.hxx>
#include <fmtanchr.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtfollowtextflow.hxx> // #i30669#
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <pagefrm.hxx>
#include <frmatr.hxx>
#include <fmtcntnt.hxx>
#include <ndindex.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <swrect.hxx>
#include <ndgrf.hxx>
#include <grfatr.hxx>
#include <ndole.hxx>
#include <unodraw.hxx>
#include <pagedesc.hxx>
#include <ww8par.hxx>
#include <breakit.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include "ww8attributeoutput.hxx"
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "wrtww8.hxx"
#include "escher.hxx"
#include <ndtxt.hxx>
#include "WW8FFData.hxx"
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include "docsh.hxx"
#include <oox/ole/olehelper.hxx>
#include <fstream>
#include <unotools/streamwrap.hxx>


using ::editeng::SvxBorderLine;
using namespace com::sun::star;
using namespace sw::util;
using namespace sw::types;
using namespace nsFieldFlags;

namespace
{
    /// Get the Z ordering number for a DrawObj in a WW8Export.
    /// @param rWrt The containing WW8Export.
    /// @param pObj pointer to the drawing object.
    /// @returns The ordering number.
    static sal_uLong lcl_getSdrOrderNumber(const WW8Export& rWrt, DrawObj *pObj)
    {
        return rWrt.GetSdrOrdNum(pObj->maCntnt.GetFrmFmt());
    };

    /// A function object to act as a predicate comparing the ordering numbers
    /// of two drawing obejcts in a WW8Export.
    class CompareDrawObjs
    {
    private:
        const WW8Export& wrt;

    public:
        CompareDrawObjs(const WW8Export& rWrt) : wrt(rWrt) {};
        bool operator()(DrawObj *a, DrawObj *b) const
        {
            sal_uLong aSort = ::lcl_getSdrOrderNumber(wrt, a);
            sal_uLong bSort = ::lcl_getSdrOrderNumber(wrt, b);
            return aSort < bSort;
        }
    };

    /// Make a z-order sorted copy of a collection of DrawObj objects.
    /// @param rWrt    The containing WW8Export.
    /// @param rSrcArr The source array.
    /// @param rDstArr The destination array.
    static void lcl_makeZOrderArray(const WW8Export& rWrt,
                                    std::vector<DrawObj> &rSrcArr,
                                    std::vector<DrawObj*> &rDstArr)
    {
        rDstArr.clear();
        rDstArr.reserve(rSrcArr.size());
        for(size_t i = 0; i < rSrcArr.size(); ++i)
        {
            rDstArr.push_back( &rSrcArr[i] );
        }
        std::sort(rDstArr.begin(), rDstArr.end(), ::CompareDrawObjs(rWrt));
    }

}

// get a part fix for this type of element
bool WW8Export::MiserableFormFieldExportHack(const SwFrmFmt& rFrmFmt)
{
    OSL_ENSURE(bWrtWW8, "Not allowed");
    if (!bWrtWW8)
        return false;
    bool bHack = false;
    const SdrObject *pObject = rFrmFmt.FindRealSdrObject();
    if (pObject && pObject->GetObjInventor() == FmFormInventor)
    {
        if (SdrUnoObj *pFormObj = PTR_CAST(SdrUnoObj,pObject))
        {
            uno::Reference< awt::XControlModel > xControlModel =
                pFormObj->GetUnoControlModel();
            uno::Reference< lang::XServiceInfo > xInfo(xControlModel,
                uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xPropSet(xControlModel, uno::UNO_QUERY);
            if (xInfo->supportsService(C2U("com.sun.star.form.component.ComboBox")))
            {
                DoComboBox(xPropSet);
                bHack = true;
            }
            else if (xInfo->supportsService(C2U("com.sun.star.form.component.CheckBox")))
            {
                DoCheckBox(xPropSet);
                bHack = true;
            }
        }
    }
    return bHack;
}


void WW8Export::DoComboBox(uno::Reference<beans::XPropertySet> xPropSet)
{
    rtl::OUString sSelected;
    uno::Sequence<rtl::OUString> aListItems;
    xPropSet->getPropertyValue(C2U("StringItemList")) >>= aListItems;
    sal_Int32 nNoStrings = aListItems.getLength();
    if (nNoStrings)
    {
        uno::Any aTmp = xPropSet->getPropertyValue(C2U("DefaultText"));
        const rtl::OUString *pStr = (const rtl::OUString *)aTmp.getValue();
        if (pStr)
            sSelected = *pStr;
    }

    rtl::OUString sName;
    {
        uno::Any aTmp = xPropSet->getPropertyValue(C2U("Name"));
        const rtl::OUString *pStr = (const rtl::OUString *)aTmp.getValue();
        if (pStr)
            sName = *pStr;
    }


    rtl::OUString sHelp;
    {
        // property "Help" does not exist and due to the no-existence an exception is thrown.
        try
        {
            uno::Any aTmp = xPropSet->getPropertyValue(C2U("HelpText"));
            const rtl::OUString *pStr = (const rtl::OUString *)aTmp.getValue();
            if (pStr)
                sHelp = *pStr;
        }
        catch( const uno::Exception& )
        {}
    }

    rtl::OUString sToolTip;
    {
        uno::Any aTmp = xPropSet->getPropertyValue(C2U("Name"));
        const rtl::OUString *pStr = (const rtl::OUString *)aTmp.getValue();
        if (pStr)
            sToolTip = *pStr;
    }

    DoComboBox(sName, sHelp, sToolTip, sSelected, aListItems);
}

void WW8Export::DoComboBox(const rtl::OUString &rName,
                             const rtl::OUString &rHelp,
                             const rtl::OUString &rToolTip,
                             const rtl::OUString &rSelected,
                             uno::Sequence<rtl::OUString> &rListItems)
{
    OSL_ENSURE(bWrtWW8, "Not allowed");
    if (!bWrtWW8)
        return;
    OutputField(0, ww::eFORMDROPDOWN, FieldString(ww::eFORMDROPDOWN),
             WRITEFIELD_START | WRITEFIELD_CMD_START);
    // write the refence to the "picture" structure
    sal_uLong nDataStt = pDataStrm->Tell();
    pChpPlc->AppendFkpEntry( Strm().Tell() );

    WriteChar( 0x01 );

    static sal_uInt8 aArr1[] =
    {
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation
        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01,       // sprmCFSpec
        0x02, 0x08, 0x01        // sprmCFFldVanish
    };
    sal_uInt8* pDataAdr = aArr1 + 2;
    Set_UInt32( pDataAdr, nDataStt );

    pChpPlc->AppendFkpEntry(Strm().Tell(), sizeof(aArr1), aArr1);

    OutputField(0, ww::eFORMDROPDOWN, FieldString(ww::eFORMDROPDOWN),
             WRITEFIELD_CLOSE);

    ::sw::WW8FFData aFFData;

    aFFData.setType(2);
    aFFData.setName(rName);
    aFFData.setHelp(rHelp);
    aFFData.setStatus(rToolTip);

    sal_uInt32 nListItems = rListItems.getLength();

    for (sal_uInt32 i = 0; i < nListItems; i++)
    {
        if (i < 0x20 && rSelected == rListItems[i])
            aFFData.setResult(::sal::static_int_cast<sal_uInt8>(i));
        aFFData.addListboxEntry(rListItems[i]);
    }

    aFFData.Write(pDataStrm);
}

void WW8Export::DoCheckBox(uno::Reference<beans::XPropertySet> xPropSet)
{
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo =
        xPropSet->getPropertySetInfo();

    OutputField(0, ww::eFORMCHECKBOX, FieldString(ww::eFORMCHECKBOX),
        WRITEFIELD_START | WRITEFIELD_CMD_START);
    // write the refence to the "picture" structure
    sal_uLong nDataStt = pDataStrm->Tell();
    pChpPlc->AppendFkpEntry( Strm().Tell() );

    WriteChar( 0x01 );
    static sal_uInt8 aArr1[] = {
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation

        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01,       // sprmCFSpec
        0x02, 0x08, 0x01        // sprmCFFldVanish
    };
    sal_uInt8* pDataAdr = aArr1 + 2;
    Set_UInt32( pDataAdr, nDataStt );

    pChpPlc->AppendFkpEntry(Strm().Tell(),
                sizeof( aArr1 ), aArr1 );

    ::sw::WW8FFData aFFData;

    aFFData.setType(1);
    aFFData.setCheckboxHeight(0x14);

    sal_Int16 nTemp = 0;
    xPropSet->getPropertyValue(C2U("DefaultState")) >>= nTemp;
    aFFData.setDefaultResult(nTemp);

    xPropSet->getPropertyValue(C2U("State")) >>= nTemp;
    aFFData.setResult(nTemp);

    ::rtl::OUString aStr;
    static ::rtl::OUString sName(C2U("Name"));
    if (xPropSetInfo->hasPropertyByName(sName))
    {
        xPropSet->getPropertyValue(sName) >>= aStr;
        aFFData.setName(aStr);
    }

    static ::rtl::OUString sHelpText(C2U("HelpText"));
    if (xPropSetInfo->hasPropertyByName(sHelpText))
    {
        xPropSet->getPropertyValue(sHelpText) >>= aStr;
        aFFData.setHelp(aStr);
    }
    static ::rtl::OUString sHelpF1Text(C2U("HelpF1Text"));
    if (xPropSetInfo->hasPropertyByName(sHelpF1Text))
    {
        xPropSet->getPropertyValue(sHelpF1Text) >>= aStr;
        aFFData.setStatus(aStr);
    }

    aFFData.Write(pDataStrm);

    OutputField(0, ww::eFORMCHECKBOX, aEmptyStr, WRITEFIELD_CLOSE);
}

void WW8Export::DoFormText(const SwInputField * pFld)
{
    OutputField(0, ww::eFORMTEXT, FieldString(ww::eFORMTEXT),
        WRITEFIELD_START | WRITEFIELD_CMD_START);
    // write the refence to the "picture" structure
    sal_uLong nDataStt = pDataStrm->Tell();
    pChpPlc->AppendFkpEntry( Strm().Tell() );

    WriteChar( 0x01 );
    static sal_uInt8 aArr1[] = {
        0x02, 0x08, 0x81,        // sprmCFFldVanish
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation

        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01       // sprmCFSpec
    };
    sal_uInt8* pDataAdr = aArr1 + 5;
    Set_UInt32( pDataAdr, nDataStt );

    pChpPlc->AppendFkpEntry(Strm().Tell(),
                sizeof( aArr1 ), aArr1 );

    ::sw::WW8FFData aFFData;

    aFFData.setType(0);
    aFFData.setName(pFld->GetPar2());
    aFFData.setHelp(pFld->GetHelp());
    aFFData.setStatus(pFld->GetToolTip());
    aFFData.Write(pDataStrm);

    OutputField(0, ww::eFORMTEXT, aEmptyStr, WRITEFIELD_CMD_END);

    String const fieldStr( pFld->ExpandField(true) );
    SwWW8Writer::WriteString16(Strm(), fieldStr, false);

    static sal_uInt8 aArr2[] = {
        0x03, 0x6a, 0x00, 0x00, 0x00, 0x00, // sprmCPicLocation
        0x55, 0x08, 0x01,  // sprmCFSpec
        0x75, 0x08, 0x01       // ???
    };

    pDataAdr = aArr2 + 2;
    Set_UInt32( pDataAdr, nDataStt );
    pChpPlc->AppendFkpEntry(Strm().Tell(),
                sizeof( aArr2 ), aArr2 );

    OutputField(0, ww::eFORMTEXT, aEmptyStr, WRITEFIELD_CLOSE);
}

PlcDrawObj::~PlcDrawObj()
{
}

//Its irritating to have to change the RTL frames position into LTR ones
//so that word will have to place them in the right place. Doubly so that
//the SO drawings and writer frames have different ideas themselves as to
//how to be positioned when in RTL mode!
bool RTLGraphicsHack(SwTwips &rLeft, SwTwips nWidth,
sal_Int16 eHoriOri, sal_Int16 eHoriRel, SwTwips nPageLeft,
    SwTwips nPageRight, SwTwips nPageSize)
{
    bool bRet = false;
    if (eHoriOri == text::HoriOrientation::NONE)
    {
        if (eHoriRel == text::RelOrientation::PAGE_FRAME)
        {
            rLeft = nPageSize - rLeft;
            bRet = true;
        }
        else if (
                  (eHoriRel == text::RelOrientation::PAGE_PRINT_AREA) ||
                  (eHoriRel == text::RelOrientation::FRAME) ||
                  (eHoriRel == text::RelOrientation::PRINT_AREA)
                )
        {
            rLeft = nPageSize - nPageLeft - nPageRight - rLeft;
            bRet = true;
        }
    }
    if (bRet)
        rLeft -= nWidth;
    return bRet;
}

bool RTLDrawingsHack(long &rLeft, long /*nWidth*/,
    sal_Int16 eHoriOri, sal_Int16 eHoriRel, SwTwips nPageLeft,
    SwTwips nPageRight, SwTwips nPageSize)
{
    bool bRet = false;
    if (eHoriOri == text::HoriOrientation::NONE)
    {
        if (eHoriRel == text::RelOrientation::PAGE_FRAME)
        {
            rLeft = nPageSize + rLeft;
            bRet = true;
        }
        else if (
                  (eHoriRel == text::RelOrientation::PAGE_PRINT_AREA) ||
                  (eHoriRel == text::RelOrientation::FRAME) ||
                  (eHoriRel == text::RelOrientation::PRINT_AREA)
                )
        {
            rLeft = nPageSize - nPageLeft - nPageRight + rLeft;
            bRet = true;
        }
    }
    return bRet;
}

bool WW8Export::MiserableRTLFrmFmtHack(SwTwips &rLeft, SwTwips &rRight,
    const sw::Frame &rFrmFmt)
{
    //Require nasty bidi swap
    if (FRMDIR_HORI_RIGHT_TOP != pDoc->GetTextDirection(rFrmFmt.GetPosition()))
        return false;

    SwTwips nWidth = rRight - rLeft;
    SwTwips nPageLeft, nPageRight;
    SwTwips nPageSize = CurrentPageWidth(nPageLeft, nPageRight);

    const SwFmtHoriOrient& rHOr = rFrmFmt.GetFrmFmt().GetHoriOrient();

    bool bRet = false;
    sw::Frame::WriterSource eSource = rFrmFmt.GetWriterType();
    if (eSource == sw::Frame::eDrawing || eSource == sw::Frame::eFormControl)
    {
        if (RTLDrawingsHack(rLeft, nWidth, rHOr.GetHoriOrient(),
            rHOr.GetRelationOrient(), nPageLeft, nPageRight, nPageSize))
        {
            bRet = true;
        }
    }
    else
    {
        if (RTLGraphicsHack(rLeft, nWidth, rHOr.GetHoriOrient(),
            rHOr.GetRelationOrient(), nPageLeft, nPageRight, nPageSize))
        {
            bRet = true;
        }
    }
    if (bRet)
        rRight = rLeft + nWidth;
    return bRet;
}

void PlcDrawObj::WritePlc( WW8Export& rWrt ) const
{
    if (8 > rWrt.pFib->nVersion)    // Cannot export drawobject in vers 7-
        return;

    sal_uInt32 nFcStart = rWrt.pTableStrm->Tell();

    if (!maDrawObjs.empty())
    {
        // write CPs
        WW8Fib& rFib = *rWrt.pFib;
        WW8_CP nCpOffs = GetCpOffset(rFib);

        cDrawObjIter aEnd = maDrawObjs.end();
        cDrawObjIter aIter;

        for (aIter = maDrawObjs.begin(); aIter < aEnd; ++aIter)
            SwWW8Writer::WriteLong(*rWrt.pTableStrm, aIter->mnCp - nCpOffs);

        SwWW8Writer::WriteLong(*rWrt.pTableStrm, rFib.ccpText + rFib.ccpFtn +
            rFib.ccpHdr + rFib.ccpEdn + rFib.ccpTxbx + rFib.ccpHdrTxbx + 1);

        for (aIter = maDrawObjs.begin(); aIter < aEnd; ++aIter)
        {
            // write the fspa-struct
            const sw::Frame &rFrmFmt = aIter->maCntnt;
            const SwFrmFmt &rFmt = rFrmFmt.GetFrmFmt();
            const SdrObject* pObj = rFmt.FindRealSdrObject();

            Rectangle aRect;
            SwFmtVertOrient rVOr = rFmt.GetVertOrient();
            SwFmtHoriOrient rHOr = rFmt.GetHoriOrient();
            // #i30669# - convert the positioning attributes.
            // Most positions are converted, if layout information exists.
            const bool bPosConverted =
                WinwordAnchoring::ConvertPosition( rHOr, rVOr, rFmt );

            Point aObjPos;
            if (RES_FLYFRMFMT == rFmt.Which())
            {
                SwRect aLayRect(rFmt.FindLayoutRect(false, &aObjPos));
                // the Object is not visible - so get the values from
                // the format. The Position may not be correct.
                if( aLayRect.IsEmpty() )
                    aRect.SetSize( rFmt.GetFrmSize().GetSize() );
                else
                {
                    // #i56090# Do not only consider the first client
                    // Note that we actually would have to find the maximum size of the
                    // frame format clients. However, this already should work in most cases.
                    const SwRect aSizeRect(rFmt.FindLayoutRect());
                    if ( aSizeRect.Width() > aLayRect.Width() )
                        aLayRect.Width( aSizeRect.Width() );

                    aRect = aLayRect.SVRect();
                }
            }
            else
            {
                OSL_ENSURE(pObj, "wo ist das SDR-Object?");
                if (pObj)
                {
                    aRect = pObj->GetSnapRect();
                }
            }

            // #i30669# - use converted position, if conversion is performed.
            // Unify position determination of Writer fly frames
            // and drawing objects.
            if ( bPosConverted )
            {
                aRect.SetPos( Point( rHOr.GetPos(), rVOr.GetPos() ) );
            }
            else
            {
                aRect -= aIter->maParentPos;
                aObjPos = aRect.TopLeft();
                if (text::VertOrientation::NONE == rVOr.GetVertOrient())
                {
                    // #i22673#
                    sal_Int16 eOri = rVOr.GetRelationOrient();
                    if (eOri == text::RelOrientation::CHAR || eOri == text::RelOrientation::TEXT_LINE)
                        aObjPos.Y() = -rVOr.GetPos();
                    else
                        aObjPos.Y() = rVOr.GetPos();
                }
                if (text::HoriOrientation::NONE == rHOr.GetHoriOrient())
                    aObjPos.X() = rHOr.GetPos();
                aRect.SetPos( aObjPos );
            }

            sal_Int32 nThick = aIter->mnThick;

            //If we are being exported as an inline hack, set
            //corner to 0 and forget about border thickness for positioning
            if (rFrmFmt.IsInline())
            {
                aRect.SetPos(Point(0,0));
                nThick = 0;
            }

            // spid
            SwWW8Writer::WriteLong(*rWrt.pTableStrm, aIter->mnShapeId);

            SwTwips nLeft = aRect.Left() + nThick;
            SwTwips nRight = aRect.Right() - nThick;

            //Nasty swap for bidi if neccessary
            rWrt.MiserableRTLFrmFmtHack(nLeft, nRight, rFrmFmt);

            //xaLeft/yaTop/xaRight/yaBottom - rel. to anchor
            //(most of) the border is outside the graphic is word, so
            //change dimensions to fit
            SwWW8Writer::WriteLong(*rWrt.pTableStrm, nLeft);
            SwWW8Writer::WriteLong(*rWrt.pTableStrm,aRect.Top() + nThick);
            SwWW8Writer::WriteLong(*rWrt.pTableStrm, nRight);
            SwWW8Writer::WriteLong(*rWrt.pTableStrm,aRect.Bottom() - nThick);

            //fHdr/bx/by/wr/wrk/fRcaSimple/fBelowText/fAnchorLock
            sal_uInt16 nFlags=0;
            //If nFlags isn't 0x14 its overridden by the escher properties
            if (FLY_AT_PAGE == rFmt.GetAnchor().GetAnchorId())
                nFlags = 0x0000;
            else
                nFlags = 0x0014;        // x-rel to text,  y-rel to text

            const SwFmtSurround& rSurr = rFmt.GetSurround();
            sal_uInt16 nContour = rSurr.IsContour() ? 0x0080 : 0x0040;
            SwSurround eSurround = rSurr.GetSurround();

            /*
             #i3958#
             The inline elements being export as anchored to character inside
             the shape field hack are required to be wrap through so as to flow
             over the following dummy 0x01 graphic
            */
            if (rFrmFmt.IsInline())
                eSurround = SURROUND_THROUGHT;

            switch (eSurround)
            {
                case SURROUND_NONE:
                    nFlags |= 0x0020;
                    break;
                case SURROUND_THROUGHT:
                    nFlags |= 0x0060;
                    break;
                case SURROUND_PARALLEL:
                    nFlags |= 0x0000 | nContour;
                    break;
                case SURROUND_IDEAL:
                    nFlags |= 0x0600 | nContour;
                    break;
                case SURROUND_LEFT:
                    nFlags |= 0x0200 | nContour;
                    break;
                case SURROUND_RIGHT:
                    nFlags |= 0x0400 | nContour;
                    break;
                default:
                    OSL_ENSURE(!this, "Unsupported surround type for export");
                    break;
            }
            if (pObj && (pObj->GetLayer() == rWrt.pDoc->GetHellId() ||
                    pObj->GetLayer() == rWrt.pDoc->GetInvisibleHellId()))
            {
                nFlags |= 0x4000;
            }

            /*
             #i3958# Required to make this inline stuff work in WordXP, not
             needed for 2003 interestingly
             */
            if (rFrmFmt.IsInline())
                nFlags |= 0x8000;

            SwWW8Writer::WriteShort(*rWrt.pTableStrm, nFlags);

            // cTxbx
            SwWW8Writer::WriteLong(*rWrt.pTableStrm, 0);
        }

        RegisterWithFib(rFib, nFcStart, rWrt.pTableStrm->Tell() - nFcStart);
    }
}

void MainTxtPlcDrawObj::RegisterWithFib(WW8Fib &rFib, sal_uInt32 nStart,
    sal_uInt32 nLen) const
{
    rFib.fcPlcfspaMom = nStart;
    rFib.lcbPlcfspaMom = nLen;
}

WW8_CP MainTxtPlcDrawObj::GetCpOffset(const WW8Fib &) const
{
    return 0;
}

void HdFtPlcDrawObj::RegisterWithFib(WW8Fib &rFib, sal_uInt32 nStart,
    sal_uInt32 nLen) const
{
    rFib.fcPlcfspaHdr = nStart;
    rFib.lcbPlcfspaHdr = nLen;
}

WW8_CP HdFtPlcDrawObj::GetCpOffset(const WW8Fib &rFib) const
{
    return rFib.ccpText + rFib.ccpFtn;
}

DrawObj& DrawObj::operator=(const DrawObj& rOther)
{
    mnCp = rOther.mnCp;
    mnShapeId = rOther.mnShapeId;
    maCntnt = rOther.maCntnt;
    maParentPos = rOther.maParentPos;
    mnThick = rOther.mnThick;
    mnDirection = rOther.mnDirection;
    mnHdFtIndex = rOther.mnHdFtIndex;
    return *this;
}

bool PlcDrawObj::Append( WW8Export& rWrt, WW8_CP nCp, const sw::Frame& rFmt,
    const Point& rNdTopLeft )
{
    bool bRet = false;
    const SwFrmFmt &rFormat = rFmt.GetFrmFmt();
    if (TXT_HDFT == rWrt.nTxtTyp || TXT_MAINTEXT == rWrt.nTxtTyp)
    {
        if (RES_FLYFRMFMT == rFormat.Which())
        {
            // check for textflyframe and if it is the first in a Chain
            if (rFormat.GetCntnt().GetCntntIdx())
                bRet = true;
        }
        else
            bRet = true;
    }

    if (bRet)
    {
        DrawObj aObj(rFmt, nCp, rNdTopLeft, rWrt.TrueFrameDirection(rFormat),
            rWrt.GetHdFtIndex());
        maDrawObjs.push_back(aObj);
    }
    return bRet;
}

void DrawObj::SetShapeDetails(sal_uInt32 nId, sal_Int32 nThick)
{
    mnShapeId = nId;
    mnThick = nThick;
}

bool WW8_WrPlcTxtBoxes::WriteTxt( WW8Export& rWrt )
{
    bool bRet = false;
    rWrt.bInWriteEscher = true;
    WW8_CP& rccp=TXT_TXTBOX == nTyp ? rWrt.pFib->ccpTxbx : rWrt.pFib->ccpHdrTxbx;

    bRet = WriteGenericTxt( rWrt, nTyp, rccp );

    WW8_CP nCP = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    WW8Fib& rFib = *rWrt.pFib;
    WW8_CP nMyOffset = rFib.ccpText + rFib.ccpFtn + rFib.ccpHdr + rFib.ccpAtn
                            + rFib.ccpEdn;
    if( TXT_TXTBOX == nTyp )
        rWrt.pFldTxtBxs->Finish( nCP, nMyOffset );
    else
        rWrt.pFldHFTxtBxs->Finish( nCP, nMyOffset + rFib.ccpTxbx );
    rWrt.bInWriteEscher = false;
    return bRet;
}

void WW8_WrPlcTxtBoxes::Append( const SdrObject& rObj, sal_uInt32 nShapeId )
{
    aCntnt.push_back( &rObj );
    aShapeIds.push_back( nShapeId );
}

const std::vector<sal_uInt32>* WW8_WrPlcTxtBoxes::GetShapeIdArr() const
{
    return &aShapeIds;
}


sal_uInt32 WW8Export::GetSdrOrdNum( const SwFrmFmt& rFmt ) const
{
    sal_uInt32 nOrdNum;
    const SdrObject* pObj = rFmt.FindRealSdrObject();
    if( pObj )
        nOrdNum = pObj->GetOrdNum();
    else
    {
        // no Layout for this format, then recalc the ordnum
        SwFrmFmt* pFmt = (SwFrmFmt*)&rFmt;
        nOrdNum = pDoc->GetSpzFrmFmts()->GetPos( pFmt );

        const SdrModel* pModel = pDoc->GetDrawModel();
        if( pModel )
            nOrdNum += pModel->GetPage( 0 )->GetObjCount();
    }
    return nOrdNum;
}

void WW8Export::AppendFlyInFlys(const sw::Frame& rFrmFmt,
    const Point& rNdTopLeft)
{
    OSL_ENSURE(bWrtWW8, "this has gone horribly wrong");
    OSL_ENSURE(!pEscher, "der EscherStream wurde schon geschrieben!");
    if (pEscher)
        return ;
    PlcDrawObj *pDrwO;
    if (TXT_HDFT == nTxtTyp)
        pDrwO = pHFSdrObjs;
    else
        pDrwO = pSdrObjs;

    if (rFrmFmt.IsInline())
    {
        OutputField(0, ww::eSHAPE, FieldString(ww::eSHAPE),
            WRITEFIELD_START | WRITEFIELD_CMD_START | WRITEFIELD_CMD_END);
    }

    WW8_CP nCP = Fc2Cp(Strm().Tell());
    bool bSuccess = pDrwO->Append(*this, nCP, rFrmFmt, rNdTopLeft);
    OSL_ENSURE(bSuccess, "Couldn't export a graphical element!");

    if (bSuccess)
    {
        static const sal_uInt8 aSpec8[] =
        {
            0x03, 0x6a, 0, 0, 0, 0, // sprmCObjLocation
            0x55, 0x08, 1           // sprmCFSpec
        };
                                                // fSpec-Attribut true
                            // Fuer DrawObjets muss ein Spezial-Zeichen
                            // in den Text und darum ein fSpec-Attribut
        pChpPlc->AppendFkpEntry( Strm().Tell() );
        WriteChar( 0x8 );
        pChpPlc->AppendFkpEntry( Strm().Tell(), sizeof( aSpec8 ), aSpec8 );

        //Need dummy picture frame
        if (rFrmFmt.IsInline())
            OutGrf(rFrmFmt);
    }

    if (rFrmFmt.IsInline())
        OutputField(0, ww::eSHAPE, aEmptyStr, WRITEFIELD_CLOSE);
}

MSWord_SdrAttrIter::MSWord_SdrAttrIter( MSWordExportBase& rWr,
    const EditTextObject& rEditObj, sal_uInt8 nTyp )
    : MSWordAttrIter( rWr ), pEditObj(&rEditObj), pEditPool(0), mnTyp(nTyp)
{
    NextPara( 0 );
}

void MSWord_SdrAttrIter::NextPara( sal_uInt16 nPar )
{
    nPara = nPar;
    // Attributwechsel an Pos 0 wird ignoriert, da davon ausgegangen
    // wird, dass am Absatzanfang sowieso die Attribute neu ausgegeben
    // werden.
    aChrTxtAtrArr.clear();
    aChrSetArr.clear();
    nAktSwPos = nTmpSwPos = 0;

    SfxItemSet aSet( pEditObj->GetParaAttribs( nPara ));
    pEditPool = aSet.GetPool();
    eNdChrSet = ItemGet<SvxFontItem>(aSet,EE_CHAR_FONTINFO).GetCharSet();

    if( pBreakIt->GetBreakIter().is() )
        nScript = pBreakIt->GetBreakIter()->getScriptType( pEditObj->GetText(nPara), 0);
    else
        nScript = i18n::ScriptType::LATIN;

    pEditObj->GetCharAttribs( nPara, aTxtAtrArr );
    nAktSwPos = SearchNext( 1 );
}

rtl_TextEncoding MSWord_SdrAttrIter::GetNextCharSet() const
{
    if( !aChrSetArr.empty() )
        return aChrSetArr.back();
    return eNdChrSet;
}

// der erste Parameter in SearchNext() liefert zurueck, ob es ein TxtAtr ist.
xub_StrLen MSWord_SdrAttrIter::SearchNext( xub_StrLen nStartPos )
{
    sal_uInt16 nMinPos = STRING_MAXLEN;
    for(std::vector<EECharAttrib>::const_iterator i = aTxtAtrArr.begin(); i < aTxtAtrArr.end(); ++i)
    {
        sal_uInt16 nPos = i->nStart; // gibt erstes Attr-Zeichen
        if( nPos >= nStartPos && nPos <= nMinPos )
        {
            nMinPos = nPos;
            SetCharSet(*i, true);
        }

        nPos = i->nEnd;              // gibt letztes Attr-Zeichen + 1
        if( nPos >= nStartPos && nPos < nMinPos )
        {
            nMinPos = nPos;
            SetCharSet(*i, false);
        }
    }
    return nMinPos;
}

void MSWord_SdrAttrIter::SetCharSet(const EECharAttrib& rAttr, bool bStart)
{
    const SfxPoolItem& rItem = *rAttr.pAttr;
    if( rItem.Which() != EE_CHAR_FONTINFO )
    {
        return;
    }

    if( bStart )
    {
        rtl_TextEncoding eChrSet = ((SvxFontItem&)rItem).GetCharSet();
        aChrSetArr.push_back( eChrSet );
        aChrTxtAtrArr.push_back( &rAttr );
    }
    else
    {
        std::vector<const EECharAttrib*>::iterator it =
           std::find( aChrTxtAtrArr.begin(), aChrTxtAtrArr.end(), &rAttr );
        if ( it != aChrTxtAtrArr.end() )
        {
            aChrTxtAtrArr.erase( it );
            aChrSetArr.erase( aChrSetArr.begin() + (it - aChrTxtAtrArr.begin()) );
        }
    }
}

void MSWord_SdrAttrIter::OutEEField(const SfxPoolItem& rHt)
{
    const SvxFieldItem &rField = (const SvxFieldItem &)rHt;
    const SvxFieldData *pFld = rField.GetField();
    if (pFld && pFld->ISA(SvxURLField))
    {
        sal_uInt8 nOldTxtTyp = m_rExport.nTxtTyp;
        m_rExport.nTxtTyp = mnTyp;
        const SvxURLField *pURL = (const SvxURLField *)pFld;
        m_rExport.AttrOutput().StartURL( pURL->GetURL(), pURL->GetTargetFrame() );

        const String &rStr = pURL->GetRepresentation();
        m_rExport.AttrOutput().RawText( rStr, true, GetNodeCharSet() ); // FIXME kendy: is the 'true' actually correct here?  It was here before, but... ;-)

        m_rExport.AttrOutput().EndURL();
        m_rExport.nTxtTyp = nOldTxtTyp;
    }
}

void MSWord_SdrAttrIter::OutAttr( xub_StrLen nSwPos )
{
    OutParaAttr(true);

    if(!aTxtAtrArr.empty())
    {
        const SwModify* pOldMod = m_rExport.pOutFmtNode;
        m_rExport.pOutFmtNode = 0;

        const SfxItemPool* pSrcPool = pEditPool;
        const SfxItemPool& rDstPool = m_rExport.pDoc->GetAttrPool();

        nTmpSwPos = nSwPos;
        sal_uInt16 nWhich, nSlotId;
        for(std::vector<EECharAttrib>::const_iterator i = aTxtAtrArr.begin(); i < aTxtAtrArr.end(); ++i)
        {
            if (nSwPos >= i->nStart && nSwPos < i->nEnd)
            {
                nWhich = i->pAttr->Which();
                if (nWhich == EE_FEATURE_FIELD)
                {
                    OutEEField(*(i->pAttr));
                    continue;
                }
                else if (nWhich == EE_FEATURE_TAB)
                {
                    m_rExport.WriteChar(0x9);
                    continue;
                }
                nSlotId = pSrcPool->GetSlotId(nWhich);

                if (nSlotId && nWhich != nSlotId)
                {
                    nWhich = rDstPool.GetWhich(nSlotId);
                    if (nWhich && nWhich != nSlotId &&
                        nWhich < RES_UNKNOWNATR_BEGIN &&
                        m_rExport.CollapseScriptsforWordOk(nScript,nWhich))
                    {
                        // use always the SW-Which Id !
                        SfxPoolItem* pI = i->pAttr->Clone();
                        pI->SetWhich( nWhich );
                        m_rExport.AttrOutput().OutputItem( *pI );
                        delete pI;
                    }
                }
            }

            if( nSwPos < i->nStart )
                break;
        }

        nTmpSwPos = 0;      // HasTextItem nur in dem obigen Bereich erlaubt
        m_rExport.pOutFmtNode = pOldMod;
    }
}

bool MSWord_SdrAttrIter::IsTxtAttr(xub_StrLen nSwPos)
{
    for (std::vector<EECharAttrib>::const_iterator i = aTxtAtrArr.begin(); i < aTxtAtrArr.end(); ++i)
    {
        if (nSwPos >= i->nStart && nSwPos < i->nEnd)
        {
            if (i->pAttr->Which() == EE_FEATURE_FIELD ||
                i->pAttr->Which() == EE_FEATURE_TAB)
                return true;
        }
    }
    return false;
}

// HasItem ist fuer die Zusammenfassung des Doppel-Attributes Underline
// und WordLineMode als TextItems. OutAttr() ruft die Ausgabefunktion,
// die dann ueber HasItem() nach anderen Items an der
// Attribut-Anfangposition fragen kann.
// Es koennen nur Attribute mit Ende abgefragt werden.
// Es wird mit bDeep gesucht
const SfxPoolItem* MSWord_SdrAttrIter::HasTextItem(sal_uInt16 nWhich) const
{
    nWhich = sw::hack::TransformWhichBetweenPools(*pEditPool,
        m_rExport.pDoc->GetAttrPool(), nWhich);
    if (nWhich)
    {
        for (std::vector<EECharAttrib>::const_iterator i = aTxtAtrArr.begin(); i < aTxtAtrArr.end(); ++i)
        {
            if (nWhich == i->pAttr->Which() && nTmpSwPos >= i->nStart && nTmpSwPos < i->nEnd)
                return i->pAttr;    // Found
            else if (nTmpSwPos < i->nStart)
                return NULL;        // dann kommt da nichts mehr
        }
    }
    return NULL;
}

const SfxPoolItem& MSWord_SdrAttrIter::GetItem( sal_uInt16 nWhich ) const
{
    using sw::hack::GetSetWhichFromSwDocWhich;
    const SfxPoolItem* pRet = HasTextItem(nWhich);
    if (!pRet)
    {
        SfxItemSet aSet(pEditObj->GetParaAttribs(nPara));
        nWhich = GetSetWhichFromSwDocWhich(aSet, *m_rExport.pDoc, nWhich);
        OSL_ENSURE(nWhich, "Impossible, catastrophic failure imminent");
        pRet = &aSet.Get(nWhich);
    }
    return *pRet;
}

void MSWord_SdrAttrIter::OutParaAttr(bool bCharAttr)
{
    SfxItemSet aSet( pEditObj->GetParaAttribs( nPara ));
    if( aSet.Count() )
    {
        const SfxItemSet* pOldSet = m_rExport.GetCurItemSet();
        m_rExport.SetCurItemSet( &aSet );

        SfxItemIter aIter( aSet );
        const SfxPoolItem* pItem = aIter.GetCurItem();

        const SfxItemPool* pSrcPool = pEditPool,
                         * pDstPool = &m_rExport.pDoc->GetAttrPool();

        do {
            sal_uInt16 nWhich = pItem->Which(),
                   nSlotId = pSrcPool->GetSlotId( nWhich );

            if ( nSlotId && nWhich != nSlotId &&
                 0 != ( nWhich = pDstPool->GetWhich( nSlotId ) ) &&
                 nWhich != nSlotId &&
                 ( bCharAttr ? ( nWhich >= RES_CHRATR_BEGIN && nWhich < RES_TXTATR_END )
                             : ( nWhich >= RES_PARATR_BEGIN && nWhich < RES_FRMATR_END ) ) )
            {
                // use always the SW-Which Id !
                SfxPoolItem* pI = pItem->Clone();
                pI->SetWhich( nWhich );
                if (m_rExport.CollapseScriptsforWordOk(nScript,nWhich))
                    m_rExport.AttrOutput().OutputItem( *pI );
                delete pI;
            }
        } while( !aIter.IsAtEnd() && 0 != ( pItem = aIter.NextItem() ) );
        m_rExport.SetCurItemSet( pOldSet );
    }
}

void WW8Export::WriteSdrTextObj(const SdrObject& rObj, sal_uInt8 nTyp)
{
    const SdrTextObj* pTxtObj = PTR_CAST(SdrTextObj, &rObj);
    OSL_ENSURE(pTxtObj, "That is no SdrTextObj!");
    if (!pTxtObj)
        return;

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
        WriteOutliner(*pParaObj, nTyp);
        if( bOwnParaObj )
            delete pParaObj;
    }
}

void WW8Export::WriteOutliner(const OutlinerParaObject& rParaObj, sal_uInt8 nTyp)
{
    bool bAnyWrite = false;
    const EditTextObject& rEditObj = rParaObj.GetTextObject();
    MSWord_SdrAttrIter aAttrIter( *this, rEditObj, nTyp );

    sal_uInt16 nPara = rEditObj.GetParagraphCount();
    sal_uInt8 bNul = 0;
    for( sal_uInt16 n = 0; n < nPara; ++n )
    {
        if( n )
            aAttrIter.NextPara( n );

        rtl_TextEncoding eChrSet = aAttrIter.GetNodeCharSet();

        OSL_ENSURE( pO->empty(), " pO ist am Zeilenanfang nicht leer" );

        String aStr( rEditObj.GetText( n ));
        xub_StrLen nAktPos = 0;
        xub_StrLen nEnd = aStr.Len();
        do {
            xub_StrLen nNextAttr = aAttrIter.WhereNext();
            rtl_TextEncoding eNextChrSet = aAttrIter.GetNextCharSet();

            if( nNextAttr > nEnd )
                nNextAttr = nEnd;

            bool bTxtAtr = aAttrIter.IsTxtAttr( nAktPos );
            if( !bTxtAtr )
                OutSwString( aStr, nAktPos, nNextAttr - nAktPos,
                                true, eChrSet );

                        // Am Zeilenende werden die Attribute bis ueber das CR
                        // aufgezogen. Ausnahme: Fussnoten am Zeilenende
            if( nNextAttr == nEnd && !bTxtAtr )
                WriteCR();              // CR danach

                                            // Ausgabe der Zeichenattribute
            aAttrIter.OutAttr( nAktPos );   // nAktPos - 1 ??
            pChpPlc->AppendFkpEntry( Strm().Tell(),
                                            pO->size(), pO->data() );
            pO->clear();

                        // Ausnahme: Fussnoten am Zeilenende
            if( nNextAttr == nEnd && bTxtAtr )
                WriteCR();              // CR danach
            nAktPos = nNextAttr;
            eChrSet = eNextChrSet;
            aAttrIter.NextPos();
        }
        while( nAktPos < nEnd );

        OSL_ENSURE( pO->empty(), " pO ist am ZeilenEnde nicht leer" );

        pO->push_back( bNul );        // Style # as short
        pO->push_back( bNul );

        aAttrIter.OutParaAttr(false);

        sal_uLong nPos = Strm().Tell();
        pPapPlc->AppendFkpEntry( Strm().Tell(),
                                        pO->size(), pO->data() );
        pO->clear();
        pChpPlc->AppendFkpEntry( nPos );
    }

    bAnyWrite = 0 != nPara;
    if( !bAnyWrite )
        WriteStringAsPara( aEmptyStr );
}

void WinwordAnchoring::WriteData( EscherEx& rEx ) const
{
    //Toplevel groups get their winword extra data attached, and sub elements
    //use the defaults
    if (rEx.GetGroupLevel() <= 1)
    {
        SvStream& rSt = rEx.GetStream();
        //The last argument denotes the number of sub properties in this atom
        if (mbInline)
        {
            rEx.AddAtom(18, DFF_msofbtUDefProp, 3, 3); //Prop id is 0xF122
            rSt << (sal_uInt16)0x0390 << sal_uInt32(3);
            rSt << (sal_uInt16)0x0392 << sal_uInt32(3);
            //This sub property is required to be in the dummy inline frame as
            //well
            rSt << (sal_uInt16)0x053F << nInlineHack;
        }
        else
        {
            rEx.AddAtom(24, DFF_msofbtUDefProp, 3, 4 ); //Prop id is 0xF122
            rSt << (sal_uInt16)0x038F << mnXAlign;
            rSt << (sal_uInt16)0x0390 << mnXRelTo;
            rSt << (sal_uInt16)0x0391 << mnYAlign;
            rSt << (sal_uInt16)0x0392 << mnYRelTo;
        }
    }
}


void WW8Export::CreateEscher()
{
    SfxItemState eBackSet = pDoc->GetPageDesc(0).GetMaster().
        GetItemState(RES_BACKGROUND);
    if (pHFSdrObjs->size() || pSdrObjs->size() || SFX_ITEM_SET == eBackSet)
    {
        OSL_ENSURE( !pEscher, "wer hat den Pointer nicht geloescht?" );
        SvMemoryStream* pEscherStrm = new SvMemoryStream;
        pEscherStrm->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        pEscher = new SwEscherEx(pEscherStrm, *this);
    }
}

void WW8Export::WriteEscher()
{
    if (pEscher)
    {
        sal_uLong nStart = pTableStrm->Tell();

        pEscher->WritePictures();
        pEscher->FinishEscher();

        pFib->fcDggInfo = nStart;
        pFib->lcbDggInfo = pTableStrm->Tell() - nStart;
        delete pEscher, pEscher = 0;
    }
}

void SwEscherEx::WritePictures()
{
    if( SvStream* pPicStrm = static_cast< SwEscherExGlobal& >( *mxGlobal ).GetPictureStream() )
    {
        // set the blip - entries to the correct stream pos
        sal_Int32 nEndPos = rWrt.Strm().Tell();
        mxGlobal->SetNewBlipStreamOffset( nEndPos );

        pPicStrm->Seek( 0 );
        rWrt.Strm() << *pPicStrm;
    }
    Flush();
}


// Output- Routines for Escher Export

SwEscherExGlobal::SwEscherExGlobal()
{
}

SwEscherExGlobal::~SwEscherExGlobal()
{
}

SvStream* SwEscherExGlobal::ImplQueryPictureStream()
{
    // this function will be called exactly once
    mxPicStrm.reset( new SvMemoryStream );
    mxPicStrm->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    return mxPicStrm.get();
}

SwBasicEscherEx::SwBasicEscherEx(SvStream* pStrm, WW8Export& rWW8Wrt)
    : EscherEx( EscherExGlobalRef( new SwEscherExGlobal ), pStrm), rWrt(rWW8Wrt), pEscherStrm(pStrm)
{
    Init();
}

SwBasicEscherEx::~SwBasicEscherEx()
{
}

void SwBasicEscherEx::WriteFrmExtraData(const SwFrmFmt&)
{
    AddAtom(4, ESCHER_ClientAnchor);
    GetStream() << (sal_uInt32)0x80000000;
}

void SwBasicEscherEx::WriteEmptyFlyFrame(const SwFrmFmt& rFmt, sal_uInt32 nShapeId)
{
    OpenContainer(ESCHER_SpContainer);
    AddShape(ESCHER_ShpInst_PictureFrame, 0xa00, nShapeId);
    // store anchor attribute
    WriteFrmExtraData(rFmt);

    AddAtom(6, DFF_msofbtUDefProp, 3, 1); //Prop id is 0xF122
    GetStream() << (sal_uInt16)0x053F << nInlineHack;

    CloseContainer();   // ESCHER_SpContainer
}

sal_uInt32 AddMirrorFlags(sal_uInt32 nFlags, const SwMirrorGrf &rMirror)
{
    switch (rMirror.GetValue())
    {
        default:
        case RES_MIRROR_GRAPH_DONT:
            break;
        case RES_MIRROR_GRAPH_VERT:
            nFlags |= SHAPEFLAG_FLIPH;
            break;
        case RES_MIRROR_GRAPH_HOR:
            nFlags |= SHAPEFLAG_FLIPV;
            break;
        case RES_MIRROR_GRAPH_BOTH:
            nFlags |= SHAPEFLAG_FLIPH;
            nFlags |= SHAPEFLAG_FLIPV;
            break;

    }
    return nFlags;
}

sal_Int32 SwBasicEscherEx::WriteGrfFlyFrame(const SwFrmFmt& rFmt, sal_uInt32 nShapeId)
{
    sal_Int32 nBorderThick=0;
    SwNoTxtNode *pNd = GetNoTxtNodeFromSwFrmFmt(rFmt);
    SwGrfNode *pGrfNd = pNd ? pNd->GetGrfNode() : 0;
    OSL_ENSURE(pGrfNd, "No SwGrfNode ?, suspicious");
    if (!pGrfNd)
        return nBorderThick;

    OpenContainer( ESCHER_SpContainer );

    const SwMirrorGrf &rMirror = pGrfNd->GetSwAttrSet().GetMirrorGrf();
    AddShape(ESCHER_ShpInst_PictureFrame, AddMirrorFlags(0xa00, rMirror),
        nShapeId);

    EscherPropertyContainer aPropOpt;

    sal_uInt32 nFlags = ESCHER_BlipFlagDefault;

    if (pGrfNd->IsLinkedFile())
    {
        String sURL;
        pGrfNd->GetFileFilterNms( &sURL, 0 );

        ww::bytes aBuf;
        SwWW8Writer::InsAsString16( aBuf, sURL );
        SwWW8Writer::InsUInt16( aBuf, 0 );

        sal_uInt16 nArrLen = aBuf.size();
        sal_uInt8* pArr = new sal_uInt8[ nArrLen ];
        std::copy( aBuf.begin(), aBuf.end(), pArr);

        aPropOpt.AddOpt(ESCHER_Prop_pibName, true, nArrLen, pArr, nArrLen);
        nFlags = ESCHER_BlipFlagLinkToFile | ESCHER_BlipFlagURL |
                    ESCHER_BlipFlagDoNotSave;
    }
    else
    {
        pGrfNd->SwapIn(true);

        Graphic         aGraphic(pGrfNd->GetGrf());
        GraphicObject   aGraphicObject( aGraphic );
        rtl::OString aUniqueId = aGraphicObject.GetUniqueID();

        if (!aUniqueId.isEmpty())
        {
            const   MapMode aMap100mm( MAP_100TH_MM );
            Size    aSize( aGraphic.GetPrefSize() );

            if ( MAP_PIXEL == aGraphic.GetPrefMapMode().GetMapUnit() )
            {
                aSize = Application::GetDefaultDevice()->PixelToLogic(
                    aSize, aMap100mm );
            }
            else
            {
                aSize = OutputDevice::LogicToLogic( aSize,
                    aGraphic.GetPrefMapMode(), aMap100mm );
            }

            Point aEmptyPoint = Point();
            Rectangle aRect( aEmptyPoint, aSize );

            sal_uInt32 nBlibId = mxGlobal->GetBlibID( *QueryPictureStream(),
                aUniqueId, aRect, NULL, 0 );
            if (nBlibId)
                aPropOpt.AddOpt(ESCHER_Prop_pib, nBlibId, sal_True);
        }
    }

    aPropOpt.AddOpt( ESCHER_Prop_pibFlags, nFlags );
    nBorderThick = WriteFlyFrameAttr(rFmt,mso_sptPictureFrame,aPropOpt);
    WriteGrfAttr(*pGrfNd, aPropOpt);

    aPropOpt.Commit( GetStream() );

    // store anchor attribute
    WriteFrmExtraData( rFmt );

    CloseContainer();   // ESCHER_SpContainer
    return nBorderThick;
}

void SwBasicEscherEx::WriteGrfAttr(const SwNoTxtNode& rNd,
    EscherPropertyContainer& rPropOpt)
{
    const SfxPoolItem* pItem;
    sal_uInt32 nMode = GRAPHICDRAWMODE_STANDARD;
    sal_Int32 nContrast = 0;
    sal_Int16 nBrightness = 0;

    if (SFX_ITEM_SET == rNd.GetSwAttrSet().GetItemState(RES_GRFATR_CONTRAST,
        true, &pItem))
    {
        nContrast = ((SfxInt16Item*)pItem)->GetValue();
    }

    if (SFX_ITEM_SET == rNd.GetSwAttrSet().GetItemState(RES_GRFATR_LUMINANCE,
        true, &pItem))
    {
        nBrightness = ((SfxInt16Item*)pItem)->GetValue();
    }


    if (SFX_ITEM_SET == rNd.GetSwAttrSet().GetItemState(RES_GRFATR_DRAWMODE,
        true, &pItem))
    {
        nMode = ((SfxEnumItem*)pItem)->GetValue();
        if (nMode == GRAPHICDRAWMODE_WATERMARK)
        {
            /*
            There is no real watermark mode in word, we must use standard
            mode and modify our ones by 70% extra brightness and 70% less
            contrast. This means that unmodified default OOo watermark
            will turn back into watermark, and modified OOo watermark will
            change into a close visual representation in standardmode
            */
            nBrightness += 70;
            if (nBrightness > 100)
                nBrightness = 100;
            nContrast -= 70;
            if (nContrast < -100)
                nContrast = -100;
            nMode = GRAPHICDRAWMODE_STANDARD;
        }
    }

    if (nMode == GRAPHICDRAWMODE_GREYS)
        nMode = 0x40004;
    else if (nMode == GRAPHICDRAWMODE_MONO)
        nMode = 0x60006;
    else
        nMode = 0;
    rPropOpt.AddOpt( ESCHER_Prop_pictureActive, nMode );

    if (nContrast != 0)
    {
        nContrast+=100;
        if (nContrast == 100)
            nContrast = 0x10000;
        else if (nContrast < 100)
        {
            nContrast *= 0x10000;
            nContrast /= 100;
        }
        else if (nContrast < 200)
            nContrast = (100 * 0x10000) / (200-nContrast);
        else
            nContrast = 0x7fffffff;
        rPropOpt.AddOpt( ESCHER_Prop_pictureContrast, nContrast);
    }

    if (nBrightness != 0)
        rPropOpt.AddOpt( ESCHER_Prop_pictureBrightness, nBrightness * 327 );

    if (SFX_ITEM_SET == rNd.GetSwAttrSet().GetItemState(RES_GRFATR_CROPGRF,
        true, &pItem))
    {
        const Size aSz( rNd.GetTwipSize() );
        sal_Int32 nVal;
        if( 0 != ( nVal = ((SwCropGrf*)pItem )->GetLeft() ) )
            rPropOpt.AddOpt( ESCHER_Prop_cropFromLeft, ToFract16( nVal, aSz.Width()) );
        if( 0 != ( nVal = ((SwCropGrf*)pItem )->GetRight() ) )
            rPropOpt.AddOpt( ESCHER_Prop_cropFromRight, ToFract16( nVal, aSz.Width()));
        if( 0 != ( nVal = ((SwCropGrf*)pItem )->GetTop() ) )
            rPropOpt.AddOpt( ESCHER_Prop_cropFromTop, ToFract16( nVal, aSz.Height()));
        if( 0 != ( nVal = ((SwCropGrf*)pItem )->GetBottom() ) )
            rPropOpt.AddOpt( ESCHER_Prop_cropFromBottom, ToFract16( nVal, aSz.Height()));
    }
}

void SwBasicEscherEx::SetPicId(const SdrObject &, sal_uInt32,
    EscherPropertyContainer &)
{
}

void SwEscherEx::SetPicId(const SdrObject &rSdrObj, sal_uInt32 nShapeId,
    EscherPropertyContainer &rPropOpt)
{
    pTxtBxs->Append(rSdrObj, nShapeId);
    sal_uInt32 nPicId = pTxtBxs->Count();
    nPicId *= 0x10000;
    rPropOpt.AddOpt( ESCHER_Prop_pictureId, nPicId );
}

sal_Int32 SwBasicEscherEx::WriteOLEFlyFrame(const SwFrmFmt& rFmt, sal_uInt32 nShapeId)
{
    sal_Int32 nBorderThick = 0;
    if (const SdrObject* pSdrObj = rFmt.FindRealSdrObject())
    {
        SwNodeIndex aIdx(*rFmt.GetCntnt().GetCntntIdx(), 1);
        SwOLENode& rOLENd = *aIdx.GetNode().GetOLENode();
        sal_Int64 nAspect = rOLENd.GetAspect();

        uno::Reference < embed::XEmbeddedObject > xObj(rOLENd.GetOLEObj().GetOleRef());

        // the rectangle is used to transport the size of the object
        // the left, top corner is set to ( 0, 0 ) by default constructor,
        // if the width and height are set correctly bRectIsSet should be set to true
        awt::Rectangle aRect;
        sal_Bool bRectIsSet = sal_False;


        // TODO/LATER: should the icon size be stored in case of iconified object?
        if ( xObj.is() && nAspect != embed::Aspects::MSOLE_ICON )
        {
            try
            {
                awt::Size aSize = xObj->getVisualAreaSize( nAspect );
                aRect.Width = aSize.Width;
                aRect.Height = aSize.Height;
                bRectIsSet = sal_True;
            }
            catch( const uno::Exception& )
            {}
        }

        /*
        #i5970#
        Export floating ole2 .doc ver 8+ wmf ole2 previews as emf previews
        instead ==> allows unicode text to be preserved
        */
#ifdef OLE_PREVIEW_AS_EMF
        Graphic* pGraphic = rOLENd.GetGraphic();
#endif
        OpenContainer(ESCHER_SpContainer);

        EscherPropertyContainer aPropOpt;
        const SwMirrorGrf &rMirror = rOLENd.GetSwAttrSet().GetMirrorGrf();
        WriteOLEPicture(aPropOpt, AddMirrorFlags(0xa00 | SHAPEFLAG_OLESHAPE,
            rMirror), pGraphic ? *pGraphic : Graphic(), *pSdrObj, nShapeId, bRectIsSet ? &aRect : NULL );

        nBorderThick = WriteFlyFrameAttr(rFmt, mso_sptPictureFrame, aPropOpt);
        WriteGrfAttr(rOLENd, aPropOpt);
        aPropOpt.Commit(GetStream());

        // store anchor attribute
        WriteFrmExtraData( rFmt );

        CloseContainer();   // ESCHER_SpContainer
    }
    return nBorderThick;
}

void SwBasicEscherEx::WriteBrushAttr(const SvxBrushItem &rBrush,
    EscherPropertyContainer& rPropOpt)
{
    bool bSetOpacity = false;
    sal_uInt32 nOpaque = 0;
    if (const GraphicObject *pGraphicObject = rBrush.GetGraphicObject())
    {
        rtl::OString aUniqueId = pGraphicObject->GetUniqueID();
        if (!aUniqueId.isEmpty())
        {
            const Graphic &rGraphic = pGraphicObject->GetGraphic();
            Size aSize(rGraphic.GetPrefSize());
            const MapMode aMap100mm(MAP_100TH_MM);
            if (MAP_PIXEL == rGraphic.GetPrefMapMode().GetMapUnit())
            {
                aSize = Application::GetDefaultDevice()->PixelToLogic(
                    aSize, aMap100mm);
            }
            else
            {
                aSize = OutputDevice::LogicToLogic(aSize,
                    rGraphic.GetPrefMapMode(), aMap100mm);
            }

            Point aEmptyPoint = Point();
            Rectangle aRect(aEmptyPoint, aSize);

            sal_uInt32 nBlibId = mxGlobal->GetBlibID( *QueryPictureStream(),
                aUniqueId, aRect, NULL, 0);
            if (nBlibId)
                rPropOpt.AddOpt(ESCHER_Prop_fillBlip,nBlibId,sal_True);
        }

        if (0 != (nOpaque = pGraphicObject->GetAttr().GetTransparency()))
            bSetOpacity = true;

        rPropOpt.AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );
        rPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
        rPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0 );
    }
    else
    {
        sal_uInt32 nFillColor = GetColor(rBrush.GetColor(), false);
        rPropOpt.AddOpt( ESCHER_Prop_fillColor, nFillColor );
        rPropOpt.AddOpt( ESCHER_Prop_fillBackColor, nFillColor ^ 0xffffff );
        rPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100010 );

        if (0 != (nOpaque = rBrush.GetColor().GetTransparency()))
            bSetOpacity = true;
    }

    if (bSetOpacity)
    {
        nOpaque = (nOpaque * 100) / 0xFE;
        nOpaque = ((100 - nOpaque) << 16) / 100;
        rPropOpt.AddOpt(ESCHER_Prop_fillOpacity, nOpaque);
    }
}

bool lcl_isInHeader(const SwFrmFmt& rFmt)
{
    const SwFlyFrmFmt* pFlyFrmFmt = dynamic_cast<const SwFlyFrmFmt*>(&rFmt);
    if (!pFlyFrmFmt)
        return false;
    SwFlyFrm* pFlyFrm = const_cast<SwFlyFrm*>(pFlyFrmFmt->GetFrm());
    SwPageFrm* pPageFrm = pFlyFrm->FindPageFrmOfAnchor();
    SwFrm* pHeader = pPageFrm->Lower();
    if (pHeader->GetType() == FRM_HEADER)
    {
        const SwFrm* pFrm = pFlyFrm->GetAnchorFrm();
        while (pFrm)
        {
            if (pFrm == pHeader)
                return true;
            pFrm = pFrm->GetUpper();
        }
    }
    return false;
}

sal_Int32 SwBasicEscherEx::WriteFlyFrameAttr(const SwFrmFmt& rFmt,
    MSO_SPT eShapeType, EscherPropertyContainer& rPropOpt)
{
    sal_Int32 nLineWidth=0;
    const SfxPoolItem* pItem;
    bool bFirstLine = true;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_BOX, true, &pItem))
    {
        static const sal_uInt16 aExhperProp[4] =
        {
            ESCHER_Prop_dyTextTop,  ESCHER_Prop_dyTextBottom,
            ESCHER_Prop_dxTextLeft, ESCHER_Prop_dxTextRight
        };
        const SvxBorderLine* pLine;

        for( sal_uInt16 n = 0; n < 4; ++n )
            if( 0 != ( pLine = ((SvxBoxItem*)pItem)->GetLine( n )) )
            {
                if( bFirstLine )
                {
                    sal_uInt32 nLineColor = GetColor(pLine->GetColor(), false);
                    rPropOpt.AddOpt( ESCHER_Prop_lineColor, nLineColor );
                    rPropOpt.AddOpt( ESCHER_Prop_lineBackColor,
                        nLineColor ^ 0xffffff );

                    MSO_LineStyle eStyle;
                    if( pLine->isDouble() )
                    {
                        // double line
                        nLineWidth = pLine->GetWidth();
                        if( pLine->GetInWidth() == pLine->GetOutWidth() )
                            eStyle = mso_lineDouble;
                        else if( pLine->GetInWidth() < pLine->GetOutWidth() )
                            eStyle = mso_lineThickThin;
                        else
                            eStyle = mso_lineThinThick;
                    }
                    else
                    {
                        // simple line
                        eStyle = mso_lineSimple;
                        nLineWidth = pLine->GetWidth();
                    }

                    rPropOpt.AddOpt( ESCHER_Prop_lineStyle, eStyle );
                    rPropOpt.AddOpt( ESCHER_Prop_lineWidth,
                        DrawModelToEmu( nLineWidth ));

                    MSO_LineDashing eDashing = mso_lineSolid;
                    switch (pLine->GetBorderLineStyle())
                    {
                        case  table::BorderLineStyle::DASHED:
                            eDashing = mso_lineDashGEL;
                            break;
                        case table::BorderLineStyle::DOTTED:
                            eDashing = mso_lineDotGEL;
                            break;
                        case table::BorderLineStyle::SOLID:
                        default:
                            break;
                    }
                    rPropOpt.AddOpt( ESCHER_Prop_lineDashing, eDashing );
                    rPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x8000E );

                    //Use import logic to determine how much of border will go
                    //outside graphic
                    nLineWidth = SwMSDffManager::GetEscherLineMatch(
                        eStyle,eShapeType,nLineWidth);
                    bFirstLine = false;
                }
                rPropOpt.AddOpt( aExhperProp[ n ], DrawModelToEmu(
                    ((SvxBoxItem*)pItem)->GetDistance( n ) ));
            }
            else
                // MM If there is no line the distance should be set to 0
                rPropOpt.AddOpt( aExhperProp[ n ], DrawModelToEmu(0));
    }
    if( bFirstLine )                // no valid line found
    {
        rPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
        rPropOpt.AddOpt( ESCHER_Prop_dyTextTop, 0 );
        rPropOpt.AddOpt( ESCHER_Prop_dyTextBottom, 0 );
        rPropOpt.AddOpt( ESCHER_Prop_dxTextLeft, 0 );
        rPropOpt.AddOpt( ESCHER_Prop_dxTextRight, 0 );
    }

    // SwWW8ImplReader::Read_GrafLayer() imports these as opaque
    // unconditionally, so if both are true, don't export the property.
    bool bIsInHeader = lcl_isInHeader(rFmt);
    bool bIsThrought = rFmt.GetSurround().GetValue() == SURROUND_THROUGHT;

    if (bIsInHeader)
    {
        const SvxBrushItem& rBrush(rFmt.GetBackground());
        WriteBrushAttr(rBrush, rPropOpt);
    }
    else
    {
        SvxBrushItem aBrush(rWrt.TrueFrameBgBrush(rFmt));
        WriteBrushAttr(aBrush, rPropOpt);
    }

    const SdrObject* pObj = rFmt.FindRealSdrObject();

    if( pObj && (pObj->GetLayer() == GetHellLayerId() ||
        pObj->GetLayer() == GetInvisibleHellId() ) && !(bIsInHeader && bIsThrought))
    {
        rPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x200020 );
    }

    return nLineWidth;
}

sal_Int32 SwEscherEx::WriteFlyFrameAttr(const SwFrmFmt& rFmt, MSO_SPT eShapeType,
    EscherPropertyContainer& rPropOpt)
{
    sal_Int32 nLineWidth = SwBasicEscherEx::WriteFlyFrameAttr(rFmt, eShapeType,
        rPropOpt);

    /*
     These are not in SwBasicEscherEx::WriteFlyFrameAttr because inline objs
     can't do it in word and it hacks it in by stretching the graphic that
     way, perhaps we should actually draw in this space into the graphic we
     are exporting!
     */
    const SfxPoolItem* pItem;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_LR_SPACE, true, &pItem))
    {
        rPropOpt.AddOpt( ESCHER_Prop_dxWrapDistLeft,
                DrawModelToEmu( ((SvxLRSpaceItem*)pItem)->GetLeft() ) );
        rPropOpt.AddOpt( ESCHER_Prop_dxWrapDistRight,
                DrawModelToEmu( ((SvxLRSpaceItem*)pItem)->GetRight() ) );
    }
    else
    {
        rPropOpt.AddOpt( ESCHER_Prop_dxWrapDistLeft, 0 );
        rPropOpt.AddOpt( ESCHER_Prop_dxWrapDistRight, 0 );
    }

    if (SFX_ITEM_SET == rFmt.GetItemState(RES_UL_SPACE, true, &pItem))
    {
        rPropOpt.AddOpt( ESCHER_Prop_dyWrapDistTop,
                DrawModelToEmu( ((SvxULSpaceItem*)pItem)->GetUpper() ) );
        rPropOpt.AddOpt( ESCHER_Prop_dyWrapDistBottom,
                DrawModelToEmu( ((SvxULSpaceItem*)pItem)->GetLower() ) );
    }

    if (rFmt.GetSurround().IsContour())
    {
        if (const SwNoTxtNode *pNd = GetNoTxtNodeFromSwFrmFmt(rFmt))
        {
            const PolyPolygon *pPolyPoly = pNd->HasContour();
            if (pPolyPoly && pPolyPoly->Count())
            {
                Polygon aPoly(PolygonFromPolyPolygon(*pPolyPoly));
                const Size &rOrigSize = pNd->GetGraphic().GetPrefSize();
                Fraction aMapPolyX(ww::nWrap100Percent, rOrigSize.Width());
                Fraction aMapPolyY(ww::nWrap100Percent, rOrigSize.Height());
                aPoly.Scale(aMapPolyX, aMapPolyY);

                /*
                 a) stretch right bound by 15twips
                 b) shrink bottom bound to where it would have been in word
                 c) Move it to the left by 15twips

                 See the import for details
                */
                const Size &rSize = pNd->GetTwipSize();
                Fraction aMoveHack(ww::nWrap100Percent, rSize.Width());
                aMoveHack *= Fraction(15, 1);
                long nMove(aMoveHack);

                Fraction aHackX(ww::nWrap100Percent + nMove,
                        ww::nWrap100Percent);
                Fraction aHackY(ww::nWrap100Percent - nMove,
                        ww::nWrap100Percent);
                aPoly.Scale(aHackX, aHackY);

                aPoly.Move(-nMove, 0);

                SvMemoryStream aPolyDump;
                aPolyDump.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);

                sal_uInt16 nLen = aPoly.GetSize();
                aPolyDump << nLen;
                aPolyDump << nLen;
                aPolyDump << sal_uInt16(8);
                for (sal_uInt16 nI = 0; nI < nLen; ++nI)
                {
                    aPolyDump << sal_uInt32(aPoly[nI].X());
                    aPolyDump << sal_uInt32(aPoly[nI].Y());
                }

                sal_uInt16 nArrLen = msword_cast<sal_uInt16>(aPolyDump.Tell());
                void *pArr = const_cast<void *>(aPolyDump.GetData());
                //PropOpt wants to own the buffer
                aPolyDump.ObjectOwnsMemory(false);
                rPropOpt.AddOpt(DFF_Prop_pWrapPolygonVertices, false,
                    nArrLen, static_cast<sal_uInt8 *>(pArr), nArrLen);
            }
        }
    }

    return nLineWidth;
}

void SwBasicEscherEx::Init()
{
    MapUnit eMap = MAP_TWIP;
    if (SdrModel *pModel = rWrt.pDoc->GetDrawModel())
    {
        // PPT arbeitet nur mit Einheiten zu 576DPI
        // WW hingegen verwendet twips, dh. 1440DPI.
        eMap = pModel->GetScaleUnit();
    }

    // MS-DFF-Properties sind grossteils in EMU (English Metric Units) angegeben
    // 1mm=36000emu, 1twip=635emu
    Fraction aFact(360, 1);
    aFact /= GetMapFactor(MAP_100TH_MM, eMap).X();
    // create little values
    aFact = Fraction(aFact.GetNumerator(), aFact.GetDenominator());
    mnEmuMul = aFact.GetNumerator();
    mnEmuDiv = aFact.GetDenominator();

    SetHellLayerId(rWrt.pDoc->GetHellId());
}

sal_Int32 SwBasicEscherEx::ToFract16(sal_Int32 nVal, sal_uInt32 nMax) const
{
    if (nMax)
    {
        sal_Int32 nMSVal = (nVal / 65536) * nMax;
        nMSVal += (nVal * 65536 ) / nMax;
        return nMSVal;
    }
    return 0;
}

SdrLayerID SwBasicEscherEx::GetInvisibleHellId() const
{
    return rWrt.pDoc->GetInvisibleHellId();
}

void SwBasicEscherEx::WritePictures()
{
    if( SvStream* pPicStrm = static_cast< SwEscherExGlobal& >( *mxGlobal ).GetPictureStream() )
    {
        // set the blip - entries to the correct stream pos
        sal_Int32 nEndPos = pPicStrm->Tell();
        mxGlobal->WriteBlibStoreEntry(*pEscherStrm, 1, sal_True, nEndPos);

        pPicStrm->Seek(0);
        *pEscherStrm << *pPicStrm;
    }
}

SwEscherEx::SwEscherEx(SvStream* pStrm, WW8Export& rWW8Wrt)
    : SwBasicEscherEx(pStrm, rWW8Wrt),
    pTxtBxs(0)
{
    aHostData.SetClientData(&aWinwordAnchoring);
    OpenContainer( ESCHER_DggContainer );

    sal_uInt16 nColorCount = 4;
    *pStrm  << (sal_uInt16)( nColorCount << 4 )     // instance
            << (sal_uInt16)ESCHER_SplitMenuColors   // record type
            << (sal_uInt32)( nColorCount * 4 )      // size
            << (sal_uInt32)0x08000004
            << (sal_uInt32)0x08000001
            << (sal_uInt32)0x08000002
            << (sal_uInt32)0x100000f7;

    CloseContainer();   // ESCHER_DggContainer

    sal_uInt8 i = 2;     // for header/footer and the other
    PlcDrawObj *pSdrObjs = rWrt.pHFSdrObjs;
    pTxtBxs = rWrt.pHFTxtBxs;

    // if no header/footer -> skip over
    if (!pSdrObjs->size())
    {
        --i;
        pSdrObjs = rWrt.pSdrObjs;
        pTxtBxs = rWrt.pTxtBxs;
    }

    for( ; i--; pSdrObjs = rWrt.pSdrObjs, pTxtBxs = rWrt.pTxtBxs )
    {
        // "dummy char" (or any Count ?) - why? This knows only M$
        GetStream() << (sal_Char)i;

        OpenContainer( ESCHER_DgContainer );

        EnterGroup( 0 );

        sal_uLong nSecondShapeId = pSdrObjs == rWrt.pSdrObjs ? GenerateShapeId() : 0;

        // write now all Writer-/DrawObjects
        DrawObjPointerVector aSorted;
        MakeZOrderArrAndFollowIds(pSdrObjs->GetObjArr(), aSorted);

        sal_uInt32 nShapeId=0;
        DrawObjPointerIter aEnd = aSorted.end();
        for (DrawObjPointerIter aIter = aSorted.begin(); aIter != aEnd; ++aIter)
        {
            sal_Int32 nBorderThick=0;
            DrawObj *pObj = (*aIter);
            OSL_ENSURE(pObj, "impossible");
            if (!pObj)
                continue;
            const sw::Frame &rFrame = pObj->maCntnt;
            const SwFrmFmt& rFmt = rFrame.GetFrmFmt();

            switch (rFrame.GetWriterType())
            {
                case sw::Frame::eTxtBox:
                case sw::Frame::eOle:
                case sw::Frame::eGraphic:
                    nBorderThick = WriteFlyFrm(*pObj, nShapeId, aSorted);
                    break;
                case sw::Frame::eFormControl:
                    WriteOCXControl(rFmt, nShapeId = GenerateShapeId());
                    break;
                case sw::Frame::eDrawing:
                    aWinwordAnchoring.SetAnchoring(rFmt);
                    const SdrObject* pSdrObj = rFmt.FindRealSdrObject();
                    if (pSdrObj)
                    {
                        bool bSwapInPage = false;
                        if (!pSdrObj->GetPage())
                        {
                            if (SdrModel* pModel = rWrt.pDoc->GetDrawModel())
                            {
                                if (SdrPage *pPage = pModel->GetPage(0))
                                {
                                    bSwapInPage = true;
                                    (const_cast<SdrObject*>(pSdrObj))->SetPage(pPage);
                                }
                            }
                        }

                        nShapeId = AddSdrObject(*pSdrObj);

                        if (bSwapInPage)
                            (const_cast<SdrObject*>(pSdrObj))->SetPage(0);
                    }
#if OSL_DEBUG_LEVEL > 0
                    else
                        OSL_ENSURE( !this, "Where is the SDR-Object?" );
#endif
                }

            if( !nShapeId )
            {
                nShapeId = AddDummyShape();
            }

            pObj->SetShapeDetails(nShapeId, nBorderThick);
        }

        EndSdrObjectPage();         // ???? Bugfix for 74724

        if( nSecondShapeId )
        {
            OpenContainer( ESCHER_SpContainer );

            AddShape( ESCHER_ShpInst_Rectangle, 0xe00, nSecondShapeId );

            EscherPropertyContainer aPropOpt;
            const SwFrmFmt &rFmt = rWrt.pDoc->GetPageDesc(0).GetMaster();
            const SfxPoolItem* pItem = 0;
            SfxItemState eState = rFmt.GetItemState(RES_BACKGROUND, true,
                &pItem);
            if (SFX_ITEM_SET == eState && pItem)
            {
                const SvxBrushItem* pBrush = (const SvxBrushItem*)pItem;
                WriteBrushAttr(*pBrush, aPropOpt);
            }
            aPropOpt.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080008 );
            aPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
            aPropOpt.AddOpt( ESCHER_Prop_lineWidth, 0 );

            aPropOpt.Commit( *pStrm );

            AddAtom( 4, ESCHER_ClientData );
            GetStream() << static_cast<sal_Int32>(1);

            CloseContainer();   // ESCHER_SpContainer
        }
    CloseContainer();   // ESCHER_DgContainer
    }
}

SwEscherEx::~SwEscherEx()
{
}

void SwEscherEx::FinishEscher()
{
    pEscherStrm->Seek(0);
    *rWrt.pTableStrm << *pEscherStrm;
    delete pEscherStrm, pEscherStrm = 0;
}

/** method to perform conversion of positioning attributes with the help
    of corresponding layout information

    #i30669#
    Because most of the Writer object positions doesn't correspond to the
    object positions in WW8, this method converts the positioning
    attributes. For this conversion the corresponding layout information
    is needed. If no layout information exists - e.g. no layout exists - no
    conversion is performed.
    No conversion is performed for as-character anchored objects. Whose
    object positions are already treated special in method <WriteData(..)>.

    @param _iorHoriOri
    input/output parameter - containing the current horizontal position
    attributes, which are converted by this method.

    @param _iorVertOri
    input/output parameter - containing the current vertical position
    attributes, which are converted by this method.

    @param _rFrmFmt
    input parameter - frame format of the anchored object

    @return boolean, indicating, if a conversion has been performed.
*/
bool WinwordAnchoring::ConvertPosition( SwFmtHoriOrient& _iorHoriOri,
                                         SwFmtVertOrient& _iorVertOri,
                                         const SwFrmFmt& _rFrmFmt )
{
    const RndStdIds eAnchor = _rFrmFmt.GetAnchor().GetAnchorId();

    if ( (FLY_AS_CHAR == eAnchor) || (FLY_AT_FLY == eAnchor) )
    {
        // no conversion for as-character or at frame anchored objects
        return false;
    }

    // determine anchored object
    SwAnchoredObject* pAnchoredObj( 0L );
    {
        const SwContact* pContact = _rFrmFmt.FindContactObj();
        if ( pContact )
        {
            std::list<SwAnchoredObject*> aAnchoredObjs;
            pContact->GetAnchoredObjs( aAnchoredObjs );
            if ( !aAnchoredObjs.empty() )
            {
                pAnchoredObj = aAnchoredObjs.front();
            }
        }
    }
    if ( !pAnchoredObj )
    {
        // no anchored object found. Thus, the needed layout information can't
        // be determined. --> no conversion
        return false;
    }
    // no conversion for anchored drawing object, which aren't attached to an
    // anchor frame.
    // This is the case for drawing objects, which are anchored inside a page
    // header/footer of an *unused* page style.
    if ( dynamic_cast<SwAnchoredDrawObject*>(pAnchoredObj) &&
         !pAnchoredObj->GetAnchorFrm() )
    {
        return false;
    }

    bool bConverted( false );

    // determine value of attribute 'Follow text flow', because positions aligned
    // at page areas have to be converted, if it's set.
    const bool bFollowTextFlow = _rFrmFmt.GetFollowTextFlow().GetValue();

    // check, if horizontal and vertical position have to be converted due to
    // the fact, that the object is anchored at a paragraph, which has a "column
    // break before" attribute
    bool bConvDueToAnchoredAtColBreakPara( false );
    if ( ( (eAnchor == FLY_AT_PARA) || (eAnchor == FLY_AT_CHAR) ) &&
         _rFrmFmt.GetAnchor().GetCntntAnchor() &&
         _rFrmFmt.GetAnchor().GetCntntAnchor()->nNode.GetNode().IsTxtNode() )
    {
        SwTxtNode& rAnchorTxtNode =
            dynamic_cast<SwTxtNode&>(_rFrmFmt.GetAnchor().GetCntntAnchor()->nNode.GetNode());
        const SvxFmtBreakItem* pBreak = &(ItemGet<SvxFmtBreakItem>(rAnchorTxtNode, RES_BREAK));
        if ( pBreak &&
             pBreak->GetBreak() == SVX_BREAK_COLUMN_BEFORE )
        {
            bConvDueToAnchoredAtColBreakPara = true;
        }
    }

    // convert horizontal position, if needed
    {
        enum HoriConv { NO_CONV, CONV2PG, CONV2COL, CONV2CHAR };
        HoriConv eHoriConv( NO_CONV );

        // determine, if conversion has to be performed due to the position orientation
        bool bConvDueToOrientation( false );
        {
            const sal_Int16 eHOri = _iorHoriOri.GetHoriOrient();
            bConvDueToOrientation = eHOri == text::HoriOrientation::LEFT || eHOri == text::HoriOrientation::RIGHT ||
                                    eHOri == text::HoriOrientation::INSIDE || eHOri == text::HoriOrientation::OUTSIDE ||
                                    ( eHOri != text::HoriOrientation::CENTER && _iorHoriOri.IsPosToggle() );
        }

        // determine conversion type due to the position relation
        if ( bConvDueToAnchoredAtColBreakPara )
        {
            eHoriConv = CONV2PG;
        }
        else
        {
            switch ( _iorHoriOri.GetRelationOrient() )
            {
                case text::RelOrientation::PAGE_FRAME:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    if ( bConvDueToOrientation || bFollowTextFlow )
                        eHoriConv = CONV2PG;
                }
                break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_RIGHT:
                {
                    // relation not supported by WW8. Thus, conversion always needed.
                    eHoriConv = CONV2PG;
                }
                break;
                case text::RelOrientation::FRAME:
                {
                    if ( bConvDueToOrientation )
                        eHoriConv = CONV2COL;
                }
                break;
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::FRAME_LEFT:
                case text::RelOrientation::FRAME_RIGHT:
                {
                    // relation not supported by WW8. Thus, conversion always needed.
                    eHoriConv = CONV2COL;
                }
                break;
                case text::RelOrientation::CHAR:
                {
                    if ( bConvDueToOrientation )
                        eHoriConv = CONV2CHAR;
                }
                break;
                default:
                    OSL_FAIL( "<WinwordAnchoring::ConvertPosition(..)> - unknown horizontal relation" );
            }
        }
        if ( eHoriConv != NO_CONV )
        {
            _iorHoriOri.SetHoriOrient( text::HoriOrientation::NONE );
            SwTwips nPosX( 0L );
            {
                Point aPos;
                if ( eHoriConv == CONV2PG )
                {
                    _iorHoriOri.SetRelationOrient( text::RelOrientation::PAGE_FRAME );
                    // #i33818#
                    bool bRelToTableCell( false );
                    aPos = pAnchoredObj->GetRelPosToPageFrm( bFollowTextFlow,
                                                             bRelToTableCell );
                    if ( bRelToTableCell )
                    {
                        _iorHoriOri.SetRelationOrient( text::RelOrientation::PAGE_PRINT_AREA );
                    }
                }
                else if ( eHoriConv == CONV2COL )
                {
                    _iorHoriOri.SetRelationOrient( text::RelOrientation::FRAME );
                    aPos = pAnchoredObj->GetRelPosToAnchorFrm();
                }
                else if ( eHoriConv == CONV2CHAR )
                {
                    _iorHoriOri.SetRelationOrient( text::RelOrientation::CHAR );
                    aPos = pAnchoredObj->GetRelPosToChar();
                }
                // No distinction between layout directions, because of missing
                // information about WW8 in vertical layout.
                nPosX = aPos.X();
            }
            _iorHoriOri.SetPos( nPosX );
            bConverted = true;
        }
    }

    // convert vertical position, if needed
    {
        enum VertConv { NO_CONV, CONV2PG, CONV2PARA, CONV2LINE };
        VertConv eVertConv( NO_CONV );

        // determine, if conversion has to be performed due to the position orientation
        bool bConvDueToOrientation( false );
        {
            const sal_Int16 eVOri = _iorVertOri.GetVertOrient();
            bConvDueToOrientation = ( eVOri == text::VertOrientation::TOP ||
                                      eVOri == text::VertOrientation::BOTTOM ||
                                      eVOri == text::VertOrientation::CHAR_TOP ||
                                      eVOri == text::VertOrientation::CHAR_BOTTOM ||
                                      eVOri == text::VertOrientation::CHAR_CENTER ||
                                      eVOri == text::VertOrientation::LINE_TOP ||
                                      eVOri == text::VertOrientation::LINE_BOTTOM ||
                                      eVOri == text::VertOrientation::LINE_CENTER );
        }

        // determine conversion type due to the position relation
        if ( bConvDueToAnchoredAtColBreakPara )
        {
            eVertConv = CONV2PG;
        }
        else
        {
            switch ( _iorVertOri.GetRelationOrient() )
            {
                case text::RelOrientation::PAGE_FRAME:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    if ( bConvDueToOrientation || bFollowTextFlow )
                        eVertConv = CONV2PG;
                }
                break;
                case text::RelOrientation::FRAME:
                {
                    if ( bConvDueToOrientation ||
                         _iorVertOri.GetVertOrient() == text::VertOrientation::CENTER )
                    {
                        eVertConv = CONV2PARA;
                    }
                }
                break;
                case text::RelOrientation::PRINT_AREA:
                {
                    // relation not supported by WW8. Thus, conversion always needed.
                    eVertConv = CONV2PARA;
                }
                break;
                case text::RelOrientation::CHAR:
                {
                    // relation not supported by WW8. Thus, conversion always needed.
                    eVertConv = CONV2PARA;
                }
                break;
                case text::RelOrientation::TEXT_LINE:
                {
                    if ( bConvDueToOrientation ||
                         _iorVertOri.GetVertOrient() == text::VertOrientation::NONE )
                    {
                        eVertConv = CONV2LINE;
                    }
                }
                break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_LEFT:
                case text::RelOrientation::FRAME_RIGHT:
                default:
                    OSL_FAIL( "<WinwordAnchoring::ConvertPosition(..)> - unknown vertical relation" );
            }
        }

        if ( eVertConv != NO_CONV )
        {
            _iorVertOri.SetVertOrient( text::VertOrientation::NONE );
            SwTwips nPosY( 0L );
            {
                Point aPos;
                if ( eVertConv == CONV2PG )
                {
                    _iorVertOri.SetRelationOrient( text::RelOrientation::PAGE_FRAME );
                    // #i33818#
                    bool bRelToTableCell( false );
                    aPos = pAnchoredObj->GetRelPosToPageFrm( bFollowTextFlow,
                                                             bRelToTableCell );
                    if ( bRelToTableCell )
                    {
                        _iorVertOri.SetRelationOrient( text::RelOrientation::PAGE_PRINT_AREA );
                    }
                }
                else if ( eVertConv == CONV2PARA )
                {
                    _iorVertOri.SetRelationOrient( text::RelOrientation::FRAME );
                    aPos = pAnchoredObj->GetRelPosToAnchorFrm();
                }
                else if ( eVertConv == CONV2LINE )
                {
                    _iorVertOri.SetRelationOrient( text::RelOrientation::TEXT_LINE );
                    aPos = pAnchoredObj->GetRelPosToLine();
                }
                // No distinction between layout directions, because of missing
                // information about WW8 in vertical layout.
                nPosY = aPos.Y();
            }
            _iorVertOri.SetPos( nPosY );
            bConverted = true;
        }
    }

    return bConverted;
}

void WinwordAnchoring::SetAnchoring(const SwFrmFmt& rFmt)
{
    const RndStdIds eAnchor = rFmt.GetAnchor().GetAnchorId();
    mbInline = (eAnchor == FLY_AS_CHAR);

    SwFmtHoriOrient rHoriOri = rFmt.GetHoriOrient();
    SwFmtVertOrient rVertOri = rFmt.GetVertOrient();

    // #i30669# - convert the positioning attributes.
    // Most positions are converted, if layout information exists.
    const bool bPosConverted = ConvertPosition( rHoriOri, rVertOri, rFmt );

    const sal_Int16 eHOri = rHoriOri.GetHoriOrient();
    const sal_Int16 eVOri = rVertOri.GetVertOrient(); // #i22673#

    const sal_Int16 eHRel = rHoriOri.GetRelationOrient();
    const sal_Int16 eVRel = rVertOri.GetRelationOrient();

    // horizontal Adjustment
    switch (eHOri)
    {
        default:
        case text::HoriOrientation::NONE:
            mnXAlign = 0;
            break;
        case text::HoriOrientation::LEFT:
            mnXAlign = 1;
            break;
        case text::HoriOrientation::CENTER:
            mnXAlign = 2;
            break;
        case text::HoriOrientation::RIGHT:
            mnXAlign = 3;
            break;
        case text::HoriOrientation::INSIDE:
            mnXAlign = 4;
            break;
        case text::HoriOrientation::OUTSIDE:
            mnXAlign = 5;
            break;
    }

    // vertical Adjustment
    // #i22673#
    // When adjustment is vertically relative to line or to char
    // bottom becomes top and vice versa
    const bool bVertSwap = !bPosConverted &&
                           ( (eVRel == text::RelOrientation::CHAR) ||
                             (eVRel == text::RelOrientation::TEXT_LINE) );
    switch (eVOri)
    {
        default:
        case text::VertOrientation::NONE:
            mnYAlign = 0;
            break;
        case text::VertOrientation::TOP:
        case text::VertOrientation::LINE_TOP:
        case text::VertOrientation::CHAR_TOP:
            mnYAlign = bVertSwap ? 3 : 1;
            break;
        case text::VertOrientation::CENTER:
        case text::VertOrientation::LINE_CENTER:
            mnYAlign = 2;
            break;
        case text::VertOrientation::BOTTOM:
        case text::VertOrientation::LINE_BOTTOM:
        case text::VertOrientation::CHAR_BOTTOM:
            mnYAlign = bVertSwap ? 1 : 3;
            break;
    }

    // Adjustment is horizontally relative to...
    switch (eHRel)
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            mnXRelTo = 0;
            break;
        case text::RelOrientation::PAGE_FRAME:
        case text::RelOrientation::PAGE_LEFT:  //:-(
        case text::RelOrientation::PAGE_RIGHT: //:-(
            mnXRelTo = 1;
            break;
        case text::RelOrientation::FRAME:
        case text::RelOrientation::FRAME_LEFT: //:-(
        case text::RelOrientation::FRAME_RIGHT: //:-(
            if (eAnchor == FLY_AT_PAGE)
                mnXRelTo = 1;
            else
                mnXRelTo = 2;
            break;
        case text::RelOrientation::PRINT_AREA:
            if (eAnchor == FLY_AT_PAGE)
                mnXRelTo = 0;
            else
                mnXRelTo = 2;
            break;
        case text::RelOrientation::CHAR:
            mnXRelTo = 3;
            break;
        case text::RelOrientation::TEXT_LINE:
            break;
    }

        // Adjustment is vertically relative to...
    switch (eVRel)
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            mnYRelTo = 0;
            break;
        case text::RelOrientation::PAGE_FRAME:
            mnYRelTo = 1;
            break;
        case text::RelOrientation::PRINT_AREA:
            if (eAnchor == FLY_AT_PAGE)
                mnYRelTo = 0;
            else
                mnYRelTo = 2;
            break;
        case text::RelOrientation::FRAME:
            if (eAnchor == FLY_AT_PAGE)
                mnYRelTo = 1;
            else
                mnYRelTo = 2;
            break;
        case text::RelOrientation::CHAR:
        case text::RelOrientation::TEXT_LINE: // #i22673# - vertical alignment at top of line
        case text::RelOrientation::PAGE_LEFT:   //nonsense
        case text::RelOrientation::PAGE_RIGHT:  //nonsense
        case text::RelOrientation::FRAME_LEFT:  //nonsense
        case text::RelOrientation::FRAME_RIGHT: //nonsense
            mnYRelTo = 3;
            break;
    }
}

void SwEscherEx::WriteFrmExtraData( const SwFrmFmt& rFmt )
{
    aWinwordAnchoring.SetAnchoring(rFmt);
    aWinwordAnchoring.WriteData(*this);

    AddAtom(4, ESCHER_ClientAnchor);
    GetStream() << static_cast<sal_Int32>(0);

    AddAtom(4, ESCHER_ClientData);
    GetStream() << static_cast<sal_Int32>(1);
}

sal_Int32 SwEscherEx::WriteFlyFrm(const DrawObj &rObj, sal_uInt32 &rShapeId,
    DrawObjPointerVector &rPVec)
{
    const SwFrmFmt &rFmt = rObj.maCntnt.GetFrmFmt();

    // check for textflyframe and if it is the first in a Chain
    sal_Int32 nBorderThick = 0;
    const SwNodeIndex* pNdIdx = rFmt.GetCntnt().GetCntntIdx();
    if( pNdIdx )
    {
        SwNodeIndex aIdx( *pNdIdx, 1 );
        switch( aIdx.GetNode().GetNodeType() )
        {
        case ND_GRFNODE:
            nBorderThick = WriteGrfFlyFrame( rFmt, rShapeId = GenerateShapeId() );
            break;
        case ND_OLENODE:
            nBorderThick = WriteOLEFlyFrame( rFmt, rShapeId = GenerateShapeId() );
            break;
        default:
            if (const SdrObject* pObj = rFmt.FindRealSdrObject())
            {
                // check for the first in a Chain
                sal_uInt32 nTxtId;
                sal_uInt16 nOff = 0;
                const SwFrmFmt* pFmt = &rFmt, *pPrev;
                while( 0 != ( pPrev = pFmt->GetChain().GetPrev() ))
                {
                    ++nOff;
                    pFmt = pPrev;
                }

                rShapeId = GetFlyShapeId(rFmt, rObj.mnHdFtIndex, rPVec);
                if( !nOff )
                {
                    nTxtId = pTxtBxs->GetPos( pObj );
                    if( USHRT_MAX == nTxtId )
                    {
                        pTxtBxs->Append( *pObj, rShapeId );
                        nTxtId = pTxtBxs->Count();
                    }
                    else
                        ++nTxtId;
                }
                else
                {
                    const SdrObject* pPrevObj = pFmt->FindRealSdrObject();
                    nTxtId = pTxtBxs->GetPos( pPrevObj );
                    if( USHRT_MAX == nTxtId )
                    {
                        sal_uInt32 nPrevShapeId =
                            GetFlyShapeId(*pFmt, rObj.mnHdFtIndex, rPVec);
                        pTxtBxs->Append( *pPrevObj, nPrevShapeId );
                        nTxtId = pTxtBxs->Count();
                    }
                    else
                        ++nTxtId;
                }
                nTxtId *= 0x10000;
                nTxtId += nOff;

                nBorderThick = WriteTxtFlyFrame(rObj, rShapeId, nTxtId, rPVec);
            }
        }
    }
    return nBorderThick;
}

sal_uInt16 FindPos(const SwFrmFmt &rFmt, unsigned int nHdFtIndex,
    DrawObjPointerVector &rPVec)
{
    DrawObjPointerIter aEnd = rPVec.end();
    for (DrawObjPointerIter aIter = rPVec.begin(); aIter != aEnd; ++aIter)
    {
        const DrawObj *pObj = (*aIter);
        OSL_ENSURE(pObj, "Impossible");
        if (!pObj)
            continue;
        if (
             nHdFtIndex == pObj->mnHdFtIndex &&
             &rFmt == (&pObj->maCntnt.GetFrmFmt())
           )
        {
            return static_cast< sal_uInt16 >(aIter - rPVec.begin());
        }
    }
    return USHRT_MAX;
}

sal_Int32 SwEscherEx::WriteTxtFlyFrame(const DrawObj &rObj, sal_uInt32 nShapeId,
    sal_uInt32 nTxtBox, DrawObjPointerVector &rPVec)
{
    const SwFrmFmt &rFmt = rObj.maCntnt.GetFrmFmt();
    short nDirection = rObj.mnDirection;

    sal_Int32 nBorderThick=0;
    OpenContainer( ESCHER_SpContainer );

    AddShape( ESCHER_ShpInst_TextBox, 0xa00, nShapeId );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt(ESCHER_Prop_lTxid, nTxtBox);
    if (const SwFrmFmt *pNext = rFmt.GetChain().GetNext())
    {
        sal_uInt16 nPos = FindPos(*pNext, rObj.mnHdFtIndex, rPVec);
        if (USHRT_MAX != nPos && aFollowShpIds[nPos])
            aPropOpt.AddOpt(ESCHER_Prop_hspNext, aFollowShpIds[nPos]);
    }
    nBorderThick = WriteFlyFrameAttr( rFmt, mso_sptTextBox, aPropOpt );

    MSO_TextFlow nFlow;

    switch (nDirection)
    {
        default:
            OSL_ENSURE(!this, "unknown direction type");
        case FRMDIR_HORI_LEFT_TOP:
            nFlow=mso_txflHorzN;
        break;
        case FRMDIR_HORI_RIGHT_TOP:
            nFlow=mso_txflHorzN;
        break;
        case FRMDIR_VERT_TOP_LEFT: //not really possible in word
        case FRMDIR_VERT_TOP_RIGHT:
            nFlow=mso_txflTtoBA;
        break;
    }
    aPropOpt.AddOpt( ESCHER_Prop_txflTextFlow, nFlow );

    aPropOpt.Commit( GetStream() );

    // store anchor attribute
    WriteFrmExtraData( rFmt );

    AddAtom( 4, ESCHER_ClientTextbox ); GetStream() << nTxtBox;

    CloseContainer();   // ESCHER_SpContainer
    return nBorderThick;
}

void SwBasicEscherEx::WriteOLEPicture(EscherPropertyContainer &rPropOpt,
    sal_uInt32 nShapeFlags, const Graphic &rGraphic, const SdrObject &rObj,
    sal_uInt32 nShapeId, const awt::Rectangle* pVisArea )
{
    //nShapeFlags == 0xA00 + flips and ole active
    AddShape(ESCHER_ShpInst_PictureFrame, nShapeFlags, nShapeId);

    GraphicObject aGraphicObject(rGraphic);
    rtl::OString aId = aGraphicObject.GetUniqueID();
    if (!aId.isEmpty())
    {
        Rectangle aRect = rObj.GetLogicRect();
        aRect.SetPos(Point(0,0));
        aRect.Right() = DrawModelToEmu(aRect.Right());
        aRect.Bottom() = DrawModelToEmu(aRect.Bottom());
        sal_uInt32 nBlibId = mxGlobal->GetBlibID( *QueryPictureStream(),
            aId, aRect, pVisArea, 0);    // SJ: the fourth parameter (VisArea) should be set..
        if (nBlibId)
            rPropOpt.AddOpt(ESCHER_Prop_pib, nBlibId, sal_True);
    }

    SetPicId(rObj, nShapeId, rPropOpt);
    rPropOpt.AddOpt( ESCHER_Prop_pictureActive, 0x10000 );
}

void SwEscherEx::WriteOCXControl( const SwFrmFmt& rFmt, sal_uInt32 nShapeId )
{
    if (const SdrObject* pSdrObj = rFmt.FindRealSdrObject())
    {
        OpenContainer( ESCHER_SpContainer );

        SdrModel *pModel = rWrt.pDoc->GetDrawModel();
        OutputDevice *pDevice = Application::GetDefaultDevice();
        OSL_ENSURE(pModel && pDevice, "no model or device");

        // #i71538# use complete SdrViews
        // SdrExchangeView aExchange(pModel, pDevice);
        SdrView aExchange(pModel, pDevice);

        Graphic aGraphic(aExchange.GetObjGraphic(pModel, pSdrObj));

        EscherPropertyContainer aPropOpt;
        WriteOLEPicture(aPropOpt, 0xa00 | SHAPEFLAG_OLESHAPE, aGraphic,
            *pSdrObj, nShapeId, NULL );

        WriteFlyFrameAttr( rFmt, mso_sptPictureFrame , aPropOpt );
        aPropOpt.Commit( GetStream() );

        // store anchor attribute
        WriteFrmExtraData( rFmt );

        CloseContainer();   // ESCHER_SpContainer
    }
}

void SwEscherEx::MakeZOrderArrAndFollowIds(
    std::vector<DrawObj>& rSrcArr, std::vector<DrawObj*>&rDstArr)
{
    ::lcl_makeZOrderArray(rWrt, rSrcArr, rDstArr);

    //Now set up the follow IDs
    aFollowShpIds.clear();

    for (size_t n = 0; n < rDstArr.size(); ++n)
    {
        const SwFrmFmt &rFmt = rDstArr[n]->maCntnt.GetFrmFmt();
        bool bNeedsShapeId = false;

        if (RES_FLYFRMFMT == rFmt.Which())
        {
            const SwFmtChain &rChain = rFmt.GetChain();
            if (rChain.GetPrev() || rChain.GetNext())
                bNeedsShapeId = true;
        }

        sal_uLong nShapeId = bNeedsShapeId ? GenerateShapeId() : 0;

        aFollowShpIds.push_back(nShapeId);
    }
}

sal_uInt32 SwEscherEx::GetFlyShapeId(const SwFrmFmt& rFmt,
    unsigned int nHdFtIndex, DrawObjPointerVector &rpVec)
{
    sal_uInt16 nPos = FindPos(rFmt, nHdFtIndex, rpVec);
    sal_uInt32 nShapeId;
    if (USHRT_MAX != nPos)
    {
        if (0 == (nShapeId = aFollowShpIds[nPos]))
        {
            nShapeId = GenerateShapeId();
            aFollowShpIds[ nPos ] = nShapeId;
        }
    }
    else
        nShapeId = GenerateShapeId();
    return nShapeId;
}

sal_uInt32 SwEscherEx::QueryTextID(
    const uno::Reference< drawing::XShape>& xXShapeRef, sal_uInt32 nShapeId )
{
    sal_uInt32 nId = 0;
    if (SdrObject* pObj = GetSdrObjectFromXShape(xXShapeRef))
    {
        pTxtBxs->Append( *pObj, nShapeId );
        nId = pTxtBxs->Count();
        nId *= 0x10000;
    }
    return nId;
}

SwMSConvertControls::SwMSConvertControls( SfxObjectShell *pDSh,SwPaM *pP ) : oox
::ole::MSConvertOCXControls(  pDSh ? pDSh->GetModel() : NULL ), pPaM( pP )
{
}

// in transitioning away old filter for ole/ocx controls, ReadOCXStream has been made pure virtual in
// filter/source/msocximex.cxx, so.. we need an implementation here
sal_Bool  SwMSConvertControls::ReadOCXStream( SotStorageRef& rSrc1,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShape > *pShapeRef,
        sal_Bool bFloatingCtrl )
{
    uno::Reference< form::XFormComponent > xFComp;
    sal_Bool bRes = oox::ole::MSConvertOCXControls::ReadOCXStorage( rSrc1, xFComp );
    if ( bRes && xFComp.is() )
    {
        com::sun::star::awt::Size aSz;  // not used in import
        bRes = InsertControl( xFComp, aSz,pShapeRef,bFloatingCtrl);
    }
    return bRes;
}

bool SwMSConvertControls::ExportControl(WW8Export &rWW8Wrt, const SdrObject *pObj)
{
    if (!rWW8Wrt.bWrtWW8)
        return false;

    SdrUnoObj *pFormObj = PTR_CAST(SdrUnoObj,pObj);
    uno::Reference< awt::XControlModel > xControlModel =
    pFormObj->GetUnoControlModel();

    //Why oh lord do we use so many different units ?
    //I think I painted myself into a little bit of a
    //corner by trying to use the uno interface for
    //controls export
    Rectangle aRect = pFormObj->GetLogicRect();
    aRect.SetPos(Point(0,0));
    awt::Size aSize;
    aSize.Width = TWIPS_TO_MM(aRect.Right());
    aSize.Height = TWIPS_TO_MM(aRect.Bottom());

    //Open the ObjectPool
    SvStorageRef xObjPool = rWW8Wrt.GetWriter().GetStorage().OpenSotStorage(
        rtl::OUString(SL::aObjectPool), STREAM_READWRITE |
        STREAM_SHARE_DENYALL);

    //Create a destination storage for the microsoft control
    rtl::OUStringBuffer sStorageName;
    sStorageName.append('_').append(reinterpret_cast<sal_Int64>(pObj));
    SvStorageRef xOleStg = xObjPool->OpenSotStorage(sStorageName.makeStringAndClear(),
                 STREAM_READWRITE|STREAM_SHARE_DENYALL);

    if (!xOleStg.Is())
        return false;


    rtl::OUString sUName;
    if (!WriteOCXStream( mxModel, xOleStg,xControlModel,aSize,sUName))
        return false;

    String sName = sUName;

    sal_uInt8 aSpecOLE[] =
    {
        0x03, 0x6a, 0xFF, 0xFF, 0xFF, 0xFF, // sprmCPicLocation
        0x0a, 0x08, 1,                  // sprmCFOLE2
        0x55, 0x08, 1,                  // sprmCFSpec
        0x56, 0x08, 1                   // sprmCFObj
    };
    //Set the obj id into the sprmCPicLocation
    sal_uInt8 *pData = aSpecOLE+2;
    Set_UInt32(pData,(sal_uInt32)(sal_uIntPtr)pObj);

    String sFld(FieldString(ww::eCONTROL));
    sFld.APPEND_CONST_ASC("Forms.");
    sFld += sName;
    sFld.APPEND_CONST_ASC(".1 \\s ");

    rWW8Wrt.OutputField(0, ww::eCONTROL, sFld,
        WRITEFIELD_START|WRITEFIELD_CMD_START|WRITEFIELD_CMD_END);

    rWW8Wrt.pChpPlc->AppendFkpEntry(rWW8Wrt.Strm().Tell(),sizeof(aSpecOLE),
        aSpecOLE);
    rWW8Wrt.WriteChar( 0x1 );
    rWW8Wrt.OutputField(0, ww::eCONTROL, aEmptyStr, WRITEFIELD_END | WRITEFIELD_CLOSE);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
