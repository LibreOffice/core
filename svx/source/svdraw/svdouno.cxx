/*************************************************************************
 *
 *  $RCSfile: svdouno.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2001-02-15 16:11:33 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif

#include <comphelper/processfactory.hxx>

#include "svdouno.hxx"
#include "svdxout.hxx"
#include "svdpagv.hxx"
#include "svdmodel.hxx"
#include "svdio.hxx"
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname
#include "svdetc.hxx"
#include "svdview.hxx"
#include "svdorect.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

//************************************************************
//   Defines
//************************************************************

//************************************************************
//   Hilfsklasse SdrControlEventListenerImpl
//************************************************************

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#include <cppuhelper/implbase1.hxx>

class SdrControlEventListenerImpl : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
{
protected:
    SdrUnoObj*                  pObj;

public:
    SdrControlEventListenerImpl(SdrUnoObj* _pObj)
    :   pObj(_pObj)
    {}

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    void StopListening(const uno::Reference< lang::XComponent >& xComp);
    void StartListening(const uno::Reference< lang::XComponent >& xComp);
};

// XEventListener
void SAL_CALL SdrControlEventListenerImpl::disposing( const ::com::sun::star::lang::EventObject& Source )
    throw(::com::sun::star::uno::RuntimeException)
{
    if (pObj)
    {
        pObj->xUnoControlModel = NULL;
    }
}

void SdrControlEventListenerImpl::StopListening(const uno::Reference< lang::XComponent >& xComp)
{
    if (xComp.is())
        xComp->removeEventListener(this);
}

void SdrControlEventListenerImpl::StartListening(const uno::Reference< lang::XComponent >& xComp)
{
    if (xComp.is())
        xComp->addEventListener(this);
}

//************************************************************
//   SdrUnoObj
//************************************************************

TYPEINIT1(SdrUnoObj, SdrRectObj);

SdrUnoObj::SdrUnoObj(const String& rModelName, BOOL _bOwnUnoControlModel)
:   bOwnUnoControlModel(_bOwnUnoControlModel)
{
    bIsUnoObj = TRUE;

    pEventListener = new SdrControlEventListenerImpl(this);
    pEventListener->acquire();

    // nur ein owner darf eigenstaendig erzeugen
    if (rModelName.Len())
        CreateUnoControlModel(rModelName);
}

SdrUnoObj::~SdrUnoObj()
{
    uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
    if (xComp.is())
    {
        // gehoert das Control seiner Umgebung?
        uno::Reference< container::XChild > xContent(xUnoControlModel, uno::UNO_QUERY);
        if (xContent.is() && !xContent->getParent().is())
            xComp->dispose();
        else
            pEventListener->StopListening(xComp);
    }
    pEventListener->release();
}

void SdrUnoObj::SetModel(SdrModel* pNewModel)
{
    SdrRectObj::SetModel(pNewModel);
}

void SdrUnoObj::SetPage(SdrPage* pNewPage)
{
    SdrRectObj::SetPage(pNewPage);
}

void SdrUnoObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed        =   FALSE;
    rInfo.bRotate90Allowed          =   FALSE;
    rInfo.bMirrorFreeAllowed        =   FALSE;
    rInfo.bMirror45Allowed          =   FALSE;
    rInfo.bMirror90Allowed          =   FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed             =   FALSE;
    rInfo.bEdgeRadiusAllowed        =   FALSE;
    rInfo.bNoOrthoDesired           =   FALSE;
    rInfo.bCanConvToPath            =   FALSE;
    rInfo.bCanConvToPoly            =   FALSE;
    rInfo.bCanConvToPathLineToArea  =   FALSE;
    rInfo.bCanConvToPolyLineToArea  =   FALSE;
    rInfo.bCanConvToContour = FALSE;
}

UINT16 SdrUnoObj::GetObjIdentifier() const
{
    return UINT16(OBJ_UNO);
}

FASTBOOL SdrUnoObj::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    const SdrPageView* pPV = rInfoRec.pPV;
    OutputDevice* pOut = rXOut.GetOutDev();
    OutDevType eOutDevType = pOut->GetOutDevType();
    const SdrUnoControlRec* pControlRec = NULL;

    if (pPV && xUnoControlModel.is())
    {
        const SdrPageViewWinList& rWL = pPV->GetWinList();
        USHORT nWinNum = rWL.Find(pOut);

        if (nWinNum == SDRPAGEVIEWWIN_NOTFOUND && eOutDevType == OUTDEV_VIRDEV)
        {
            // Controls koennen sich z.Z. noch nicht ins VDev zeichnen,
            // daher wird das korrespondierende, im ersten Window liegende
            // Control invalidiert (s.u.)
            if (rWL.GetCount() > 0)
            {
                // Liste enhaelt Windows, daher nehmen wir das erste
                nWinNum = 0;
            }
        }

        if (nWinNum != SDRPAGEVIEWWIN_NOTFOUND)
        {
            const SdrPageViewWinRec& rWR = rWL[nWinNum];
            const SdrUnoControlList& rControlList = rWR.GetControlList();
            USHORT nCtrlNum = rControlList.Find(xUnoControlModel);

            if (nCtrlNum != SDRUNOCONTROL_NOTFOUND)
            {
                pControlRec = &rControlList[nCtrlNum];
            }
        }
    }

    if (pControlRec && pControlRec->GetControl().is())
    {
        uno::Reference< awt::XControl > xUnoControl = pControlRec->GetControl();

        uno::Reference< awt::XView > xView(xUnoControl, uno::UNO_QUERY);
        if (xView.is())
        {
            OutputDevice* pOut = rXOut.GetOutDev();
            const MapMode& rMap = pOut->GetMapMode();
            xView->setZoom((float) double(rMap.GetScaleX()),
                           (float) double(rMap.GetScaleY()));

            BOOL bDesignMode = pPV->GetView().IsDesignMode();

            if (eOutDevType != OUTDEV_VIRDEV)
            {
                uno::Reference< awt::XWindow > xWindow(xUnoControl, uno::UNO_QUERY);
                if (xWindow.is() && eOutDevType != OUTDEV_VIRDEV)
                {
                    Point aPixPos(pOut->LogicToPixel(aRect.TopLeft()));
                    Size aPixSize(pOut->LogicToPixel(aRect.GetSize()));
                    xWindow->setPosSize(aPixPos.X(), aPixPos.Y(),
                                              aPixSize.Width(), aPixSize.Height(),
                                              awt::PosSize::POSSIZE);
                }
            }

            BOOL bInvalidatePeer = FALSE;
            if (eOutDevType == OUTDEV_WINDOW)
            {
                // Nicht wenn an der Stelle ein 'lebendes' Control liegt
                // das sich selber zeichnet.
                if (bDesignMode || pPV->GetView().IsPrintPreview())
                {
                    if (pPV->GetView().IsPrintPreview())
                    {
                        uno::Reference< awt::XGraphics > x( pOut->CreateUnoGraphics()); // UNO3
                        xView->setGraphics( x );
                    }

                    // don't draw if we're in print preview and the control isn't printable
                    // FS - 10/06/99
                    BOOL bDrawIt = TRUE;
                    if (pPV->GetView().IsPrintPreview())
                    {
                        uno::Reference< beans::XPropertySet > xP(xUnoControl->getModel(), uno::UNO_QUERY);
                        if (xP.is())
                        {
                            uno::Reference< beans::XPropertySetInfo > xPropInfo = xP->getPropertySetInfo();
                            if( xPropInfo.is() && xPropInfo->hasPropertyByName( rtl::OUString::createFromAscii("Printable")) )
                            {
                                uno::Any aVal( xP->getPropertyValue( rtl::OUString::createFromAscii("Printable")) );
                                if( aVal.hasValue() && aVal.getValueType() == ::getCppuBooleanType() )
                                    bDrawIt = *(sal_Bool*)aVal.getValue();
                            }
                            else
                                bDrawIt = FALSE;
                        }
                        else
                            bDrawIt = FALSE;
                    }

                    if (bDrawIt)
                    {
                        if( pPV->GetView().IsFillDraft() )
                        {
                            const SfxItemSet& rSet = GetItemSet();

                            // perepare ItemSet to avoid old XOut filling
                            SfxItemSet aEmptySet(*rSet.GetPool());
                            aEmptySet.Put(XFillStyleItem(XFILL_NONE));
                            rXOut.SetFillAttr(aEmptySet);

                            rXOut.SetLineAttr(rSet);

                            rXOut.DrawRect( aRect );
                        }
                        else
                        {
                            Point aP = pOut->LogicToPixel(aRect.TopLeft());
                            xView->draw(aP.X(), aP.Y());
                        }
                    }
                }
                else if ( xUnoControl->isTransparent() )
                {
                    bInvalidatePeer = TRUE;
                }
            }
            else if (eOutDevType == OUTDEV_PRINTER)
            {
                uno::Reference< beans::XPropertySet > xP(xUnoControl->getModel(), uno::UNO_QUERY);
                if (xP.is())
                {
                    uno::Reference< beans::XPropertySetInfo > xPropInfo = xP->getPropertySetInfo();
                    if( xPropInfo.is() && xPropInfo->hasPropertyByName( rtl::OUString::createFromAscii("Printable")) )
                    {
                        uno::Any aVal( xP->getPropertyValue( rtl::OUString::createFromAscii("Printable")) );
                        if( aVal.hasValue() && aVal.getValueType() == ::getCppuBooleanType() && *(sal_Bool*)aVal.getValue() )
                        {
                            uno::Reference< awt::XGraphics > x = pOut->CreateUnoGraphics(); // UNO3
                            xView->setGraphics( x );
                            Point aP = pOut->LogicToPixel(aRect.TopLeft());
                            xView->draw(aP.X(), aP.Y());
                        }
                    }
                }
            }
            else if (!bDesignMode)
            {
                bInvalidatePeer = TRUE;
            }

            if ( bInvalidatePeer )
            {
                uno::Reference< awt::XWindowPeer > xPeer(xUnoControl->getPeer());
                if (xPeer.is())
                {
                    xPeer->invalidate(INVALIDATE_NOTRANSPARENT |
                                      INVALIDATE_CHILDREN);
                }
            }
        }
    }

    return TRUE;
}

SdrObject* SdrUnoObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return ImpCheckHit(rPnt, nTol, pVisiLayer, TRUE, TRUE);
}

void SdrUnoObj::TakeObjNameSingul(XubString& rName) const
{
    rName = ImpGetResStr(STR_ObjNameSingulUno);
}

void SdrUnoObj::TakeObjNamePlural(XubString& rName) const
{
    rName = ImpGetResStr(STR_ObjNamePluralUno);
}

void SdrUnoObj::operator = (const SdrObject& rObj)
{
    SdrRectObj::operator = (rObj);

    // Freigeben des bisherigen Models
    SetUnoControlModel(uno::Reference< awt::XControlModel >());

    aUnoControlModelTypeName = ((SdrUnoObj&) rObj).aUnoControlModelTypeName;
    aUnoControlTypeName = ((SdrUnoObj&) rObj).aUnoControlTypeName;

    // Kopieren des UnoModels
    uno::Reference< io::XPersistObject > xObj(((SdrUnoObj&) rObj).GetUnoControlModel(), uno::UNO_QUERY );

    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    if (xObj.is() && xFactory.is() )
    {
        // copy it by streaming
        // creating a pipe
        uno::Reference< io::XOutputStream > xOutPipe(xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.io.Pipe")), uno::UNO_QUERY);
        uno::Reference< io::XInputStream > xInPipe(xOutPipe, uno::UNO_QUERY);

        // creating the mark streams
        uno::Reference< io::XInputStream > xMarkIn(xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")), uno::UNO_QUERY);
        uno::Reference< io::XActiveDataSink > xMarkSink(xMarkIn, uno::UNO_QUERY);

        uno::Reference< io::XOutputStream > xMarkOut(xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.io.MarkableOutputStream")), uno::UNO_QUERY);
        uno::Reference< io::XActiveDataSource > xMarkSource(xMarkOut, uno::UNO_QUERY);

        // connect mark and sink
        uno::Reference< io::XActiveDataSink > xSink(xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")), uno::UNO_QUERY);

        // connect mark and source
        uno::Reference< io::XActiveDataSource > xSource(xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")), uno::UNO_QUERY);

        uno::Reference< io::XObjectOutputStream > xOutStrm(xSource, uno::UNO_QUERY);
        uno::Reference< io::XObjectInputStream > xInStrm(xSink, uno::UNO_QUERY);

        if (xMarkSink.is() && xMarkSource.is() && xSink.is() && xSource.is())
        {
            xMarkSink->setInputStream(xInPipe);
            xMarkSource->setOutputStream(xOutPipe);
            xSink->setInputStream(xMarkIn);
            xSource->setOutputStream(xMarkOut);

            // write the object to source
            xOutStrm->writeObject(xObj);
            xOutStrm->closeOutput();
            // read the object
            uno::Reference< awt::XControlModel > xModel(xInStrm->readObject(), uno::UNO_QUERY);
            xInStrm->closeInput();

            DBG_ASSERT(xModel.is(), "SdrUnoObj::operator =, keine Model erzeugt");

            xUnoControlModel = xModel;

            // control model muss servicename des controls enthalten
            uno::Reference< beans::XPropertySet > xSet(xUnoControlModel, uno::UNO_QUERY);
            if (xSet.is())
            {
                uno::Any aValue( xSet->getPropertyValue( rtl::OUString::createFromAscii("DefaultControl")) );
                OUString aStr;

                if( aValue >>= aStr )
                    aUnoControlTypeName = String(aStr);
            }

            uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
            if (xComp.is())
                pEventListener->StartListening(xComp);
        }
    }
}

FASTBOOL SdrUnoObj::HasSpecialDrag() const
{
    return FALSE;
}

void SdrUnoObj::VisAreaChanged(const OutputDevice* pOut)
{
    if (!xUnoControlModel.is())
        return;

    if (pOut)
    {
        // Nur dieses eine OutDev beruecksichtigen
        uno::Reference< awt::XWindow > xWindow(GetUnoControl(pOut), uno::UNO_QUERY);
        if (xWindow.is())
        {
            Rectangle aPixRect(pOut->LogicToPixel(aRect));
            xWindow->setPosSize(aPixRect.Left(), aPixRect.Top(),
                         aPixRect.GetWidth(), aPixRect.GetHeight(), awt::PosSize::POSSIZE);
        }
    }
    else if (pModel)
    {
        // Controls aller PageViews beruecksichtigen
        USHORT nLstPos = pModel->GetListenerCount();
        uno::Reference< awt::XWindow > xWindow;
        Point aPixPos;
        Size aPixSize;
        SfxListener* pListener;
        SdrPageView* pPV;
        const SdrUnoControlRec* pControlRec = NULL;

        for (; nLstPos ;)
        {
            pListener = pModel->GetListener(--nLstPos);

            if (pListener && pListener->ISA(SdrPageView))
            {
                pPV = (SdrPageView*) pListener;
                const SdrPageViewWinList& rWL = pPV->GetWinList();
                USHORT nPos = rWL.GetCount();

                for (; nPos ; )
                {
                    // Controls aller OutDevs beruecksichtigen
                    const SdrPageViewWinRec& rWR = rWL[--nPos];
                    const SdrUnoControlList& rControlList = rWR.GetControlList();
                    USHORT nCtrlNum = rControlList.Find(xUnoControlModel);
                    pControlRec = (nCtrlNum != SDRUNOCONTROL_NOTFOUND) ? &rControlList[nCtrlNum] : NULL;
                    if (pControlRec)
                    {
                        xWindow = uno::Reference< awt::XWindow >(pControlRec->GetControl(), uno::UNO_QUERY);
                        if (xWindow.is())
                        {
                            // #62560 Pixelverschiebung weil mit einem Rechteck
                            // und nicht mit Point, Size gearbeitet wurde
                            aPixPos = rWR.GetOutputDevice()->LogicToPixel(aRect.TopLeft());
                            aPixSize = rWR.GetOutputDevice()->LogicToPixel(aRect.GetSize());
                            xWindow->setPosSize(aPixPos.X(), aPixPos.Y(),
                                                aPixSize.Width(), aPixSize.Height(),
                                                awt::PosSize::POSSIZE);
                        }
                    }
                }
            }
        }
    }
}

void SdrUnoObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize(rRef,xFact,yFact);

    if (aGeo.nShearWink!=0 || aGeo.nDrehWink!=0)
    {
        // kleine Korrekturen
        if (aGeo.nDrehWink>=9000 && aGeo.nDrehWink<27000)
        {
            aRect.Move(aRect.Left()-aRect.Right(),aRect.Top()-aRect.Bottom());
        }

        aGeo.nDrehWink  = 0;
        aGeo.nShearWink = 0;
        aGeo.nSin       = 0.0;
        aGeo.nCos       = 1.0;
        aGeo.nTan       = 0.0;
        SetRectsDirty();
    }

    VisAreaChanged();
}

void SdrUnoObj::NbcMove(const Size& rSize)
{
    SdrRectObj::NbcMove(rSize);
    VisAreaChanged();
}

void SdrUnoObj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetSnapRect(rRect);
    VisAreaChanged();
}

void SdrUnoObj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);
    VisAreaChanged();
}

void SdrUnoObj::CreateUnoControlModel(const String& rModelName)
{
    DBG_ASSERT(!xUnoControlModel.is(), "model already exists");

    aUnoControlModelTypeName = rModelName;

    uno::Reference< awt::XControlModel >   xModel;
    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    if (aUnoControlModelTypeName.Len() && xFactory.is() )
    {
        xModel = uno::Reference< awt::XControlModel >(xFactory->createInstance(
            aUnoControlModelTypeName), uno::UNO_QUERY);

        if (xModel.is())
            SetChanged();
    }

    SetUnoControlModel(xModel);
}

void SdrUnoObj::WriteData(SvStream& rOut) const
{
    SdrRectObj::WriteData(rOut);
    SdrDownCompat aCompat(rOut, STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)

#ifdef DBG_UTIL
    aCompat.SetID("SdrUnoObj");
#endif

    if (bOwnUnoControlModel)                    // nur als besitzt des Models dieses auch schreiben
    {
        // UNICODE: rOut << aUnoControlModelTypeName;
        rOut.WriteByteString(aUnoControlModelTypeName);
    }
}

void SdrUnoObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError() != 0)
        return;

    SdrRectObj::ReadData(rHead,rIn);

    SdrDownCompat aCompat(rIn, STREAM_READ);    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)

#ifdef DBG_UTIL
    aCompat.SetID("SdrUnoObj");
#endif

    if (bOwnUnoControlModel)                    // nur als besitzt des Models dieses auch lesen
    {
        // UNICODE: rIn >> aUnoControlModelTypeName;
        rIn.ReadByteString(aUnoControlModelTypeName);

        CreateUnoControlModel(aUnoControlModelTypeName);
    }
}

void SdrUnoObj::SetUnoControlModel( uno::Reference< awt::XControlModel > xModel)
{
    if (xUnoControlModel.is())
    {
        uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
        if (xComp.is())
            pEventListener->StopListening(xComp);

        if (pModel)
        {
            SdrHint aHint(*this);
            aHint.SetKind(HINT_CONTROLREMOVED);
            pModel->Broadcast(aHint);
        }
    }

    xUnoControlModel = xModel;

    // control model muss servicename des controls enthalten
    if (xUnoControlModel.is())
    {
        uno::Reference< beans::XPropertySet > xSet(xUnoControlModel, uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Any aValue( xSet->getPropertyValue(String("DefaultControl", gsl_getSystemTextEncoding())) );
            OUString aStr;
            if( aValue >>= aStr )
                aUnoControlTypeName = String(aStr);
        }

        uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
        if (xComp.is())
            pEventListener->StartListening(xComp);

        if (pModel)
        {
            SdrHint aHint(*this);
            aHint.SetKind(HINT_CONTROLINSERTED);
            pModel->Broadcast(aHint);
        }
    }
}

uno::Reference< awt::XControl > SdrUnoObj::GetUnoControl(const OutputDevice* pOut) const
{
    uno::Reference< awt::XControl > xUnoControl;

    if (pModel && xUnoControlModel.is())
    {
        USHORT nLstCnt = pModel->GetListenerCount();

        for (USHORT nLst = 0; nLst < nLstCnt && !xUnoControl.is(); nLst++ )
        {
            // Unter allen Listenern die PageViews suchen
            SfxListener* pListener = pModel->GetListener(nLst);

            if (pListener && pListener->ISA(SdrPageView))
            {
                // PageView gefunden
                SdrPageView* pPV = (SdrPageView*) pListener;
                const SdrPageViewWinList& rWL = pPV->GetWinList();
                USHORT nWRCnt = rWL.GetCount();
                for (USHORT nWR = 0; nWR < nWRCnt && !xUnoControl.is(); nWR++)
                {
                    // Alle WinRecords der PageView untersuchen
                    const SdrPageViewWinRec& rWR = rWL[nWR];
                    if (pOut == rWR.GetOutputDevice())
                    {
                        // Richtiges OutputDevice gefunden
                        // Darin nun das Control suchen
                        const SdrUnoControlList& rControlList = rWR.GetControlList();
                        USHORT nCtrlNum = rControlList.Find(xUnoControlModel);
                        if (nCtrlNum != SDRUNOCONTROL_NOTFOUND)
                        {
                            const SdrUnoControlRec* pControlRec = &rControlList[nCtrlNum];
                            if (pControlRec && pControlRec->GetControl().is())
                            {
                                xUnoControl = pControlRec->GetControl();
                            }
                        }
                    }
                }
            }
        }
    }
    return xUnoControl;
}

OutputDevice* SdrUnoObj::GetOutputDevice(uno::Reference< awt::XControl > _xControl) const
{
    OutputDevice* pOut = NULL;
    if (pModel && xUnoControlModel.is() && _xControl.is() && _xControl->getModel() == xUnoControlModel)
    {
        USHORT nLstCnt = pModel->GetListenerCount();
        for (USHORT nLst = 0; nLst < nLstCnt && !pOut; nLst++ )
        {
            // Unter allen Listenern die PageViews suchen
            SfxListener* pListener = pModel->GetListener(nLst);
            if (pListener && pListener->ISA(SdrPageView))
            {
                // PageView gefunden
                SdrPageView* pPV = (SdrPageView*) pListener;
                if (pPV)
                {
                    const SdrPageViewWinList& rWL = pPV->GetWinList();
                    USHORT nWRCnt = rWL.GetCount();
                    for (USHORT nWR = 0; nWR < nWRCnt && !pOut; nWR++)
                    {
                        // Alle WinRecords der PageView untersuchen
                        const SdrPageViewWinRec& rWR = rWL[nWR];
                        const SdrUnoControlList& rControlList = rWR.GetControlList();
                        if (SDRUNOCONTROL_NOTFOUND != rWR.GetControlList().Find(_xControl))
                            pOut = rWR.GetOutputDevice();
                    }
                }
            }
        }
    }
    return pOut;
}


