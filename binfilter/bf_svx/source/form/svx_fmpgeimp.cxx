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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#define _EXTERN_FORMLAYER_


#ifndef _SVXERR_HXX
#include "svxerr.hxx"
#endif

#ifndef _SVX_FMUNOPGE_HXX
#include "fmpgeimp.hxx"
#endif


#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XMARKABLESTREAM_HPP_
#include <com/sun/star/io/XMarkableStream.hpp>
#endif

#ifndef _SFX_OBJSH_HXX
#include <bf_sfx2/objsh.hxx>
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif

//  #include "fmstream.hxx"
#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif
#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif
#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif


#ifndef _SVDITER_HXX //autogen
#include "svditer.hxx"
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif


#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using namespace ::binfilter::svxform;//STRIP008 using namespace ::svxform;

/*N*/ DBG_NAME(FmFormPageImpl)
//------------------------------------------------------------------------------
/*N*/ FmFormPageImpl::FmFormPageImpl(FmFormPage* _pPage)
/*N*/                :pPage(_pPage)
/*N*/ 			   ,m_bFirstActivation( sal_True )
/*N*/ {
/*N*/     DBG_CTOR(FmFormPageImpl,NULL);
/*N*/     Init();
/*N*/ }

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/*N*/ void FmFormPageImpl::Init()
/*N*/ {
/*N*/     if (pPage)
/*N*/     {
/*N*/         FmFormModel* pDrawModel = (FmFormModel*)pPage->GetModel();
/*N*/         SfxObjectShell* pObjShell = pDrawModel->GetObjectShell();
/*N*/         if( pObjShell )
/*N*/             xModel = pObjShell->GetModel();
/*N*/ 
/*N*/         // get a unique page id from the model
/*N*/         m_sPageId = pDrawModel->GetUniquePageId();
/*N*/     }
/*N*/ 
/*N*/     static const ::rtl::OUString sFormsCollectionServiceName = ::rtl::OUString::createFromAscii("com.sun.star.form.Forms");
/*N*/     xForms = Reference< ::com::sun::star::container::XNameContainer > (
/*N*/         ::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(
/*N*/         sFormsCollectionServiceName), ::com::sun::star::uno::UNO_QUERY);
/*N*/     DBG_ASSERT(xForms.is(), "FmFormPageImpl::Init : could not create a forms collection !");
/*N*/     if (!xForms.is())
/*?*/         ShowServiceNotAvailableError(NULL, sFormsCollectionServiceName, sal_True);
/*N*/ 
/*N*/     Reference< ::com::sun::star::container::XChild >  xAsChild(xForms, UNO_QUERY);
/*N*/     if (xAsChild.is())
/*N*/         xAsChild->setParent( xModel );
/*N*/ }

//------------------------------------------------------------------------------
/*N*/ FmFormPageImpl::~FmFormPageImpl()
/*N*/ {
/*N*/     xCurrentForm = NULL;
/*N*/ 
/*N*/     ::comphelper::disposeComponent(xForms);
/*N*/     DBG_DTOR(FmFormPageImpl,NULL);
/*N*/ }

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------

//------------------------------------------------------------------

//------------------------------------------------------------------------------
/*N*/ void FmFormPageImpl::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/     // anlegen eines output streams fuer UNO
/*N*/     Reference< ::com::sun::star::io::XActiveDataSource >  xSource(::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")), UNO_QUERY);
/*N*/     Reference< ::com::sun::star::io::XOutputStream >  xMarkOut(::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableOutputStream")), UNO_QUERY);
/*N*/     Reference< ::com::sun::star::io::XActiveDataSource >  xMarkSource(xMarkOut, UNO_QUERY);
/*N*/ 
/*N*/     if (xSource.is())
/*N*/     {
/*N*/         xMarkSource->setOutputStream(new ::utl::OOutputStreamWrapper(rOut));
/*N*/         xSource->setOutputStream(xMarkOut);
/*N*/ 
/*N*/         Reference< ::com::sun::star::io::XObjectOutputStream >  xOutStrm(xSource, UNO_QUERY);
/*N*/         try
/*N*/         {
/*N*/             write(xOutStrm);
/*N*/         }
/*N*/         catch(Exception&)
/*N*/         {
/*?*/             rOut.SetError( ERRCODE_CLASS_WRITE | ERRCODE_SVX_FORMS_READWRITEFAILED | ERRCODE_WARNING_MASK );
/*N*/         }
/*N*/ 
/*N*/         xOutStrm->closeOutput();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         // let's do a hack : in former versions we didn't use UNO-stream but SvStreams, so there was
/*N*/         // no possibility that we couldn't write because of the lack of stream objects. Now, with UNO, this may happen
/*N*/         // (as it happended here). But the read-methods of the older versions don't expect that (there is no flag in
/*N*/         // the fileformat as it never was neccessary). To allow the old versions the read of the documents, we fake the
/*N*/         // write : the first token usually written by our impl is an sal_Int32 giving the number of forms within the forms
/*N*/         // collection. If this number is 0, no further reading will occur.
/*?*/         sal_Int32 nDummLength = 0; rOut << nDummLength;     // this means 'no forms'
/*?*/         nDummLength = 0; rOut << nDummLength;           // this means 'no form controls"
/*?*/         // To tell the user that something went wrong we set a warning on the stream.
/*?*/         rOut.SetError( ERRCODE_CLASS_WRITE | ERRCODE_SVX_FORMS_NOIOSERVICES | ERRCODE_WARNING_MASK );
/*N*/     }
/*N*/ }

//------------------------------------------------------------------------------
/*N*/ void FmFormPageImpl::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/     // Abholen des InputStreams ueber uno
/*N*/     Reference< ::com::sun::star::io::XActiveDataSink >  xSink(::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")), UNO_QUERY);
/*N*/ 
/*N*/     // creating the mark streams
/*N*/     Reference< ::com::sun::star::io::XInputStream >  xMarkIn(::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")), UNO_QUERY);
/*N*/     Reference< ::com::sun::star::io::XActiveDataSink >  xMarkSink(xMarkIn, UNO_QUERY);
/*N*/ 
/*N*/     if (xSink.is())
/*N*/     {
/*N*/         ::utl::OInputStreamWrapper* pUnoStream = new ::utl::OInputStreamWrapper(rIn);
/*N*/         xMarkSink->setInputStream(Reference< ::com::sun::star::io::XInputStream > (pUnoStream));
/*N*/         xSink->setInputStream(xMarkIn);
/*N*/ 
/*N*/         // freigeben aller forms
/*N*/         xCurrentForm = NULL;
/*N*/ 
/*N*/         if (xForms->hasElements())
/*N*/         {
/*?*/             ::comphelper::disposeComponent(xForms);
/*?*/             Init();
/*N*/         }
/*N*/ 
/*N*/         Reference< ::com::sun::star::io::XObjectInputStream >  xInStrm(xSink,UNO_QUERY);
/*N*/         try
/*N*/         {
/*N*/             read(xInStrm);
/*N*/         }
/*N*/         catch(Exception&)
/*N*/         {
/*?*/             rIn.SetError( ERRCODE_CLASS_READ | ERRCODE_SVX_FORMS_READWRITEFAILED | ERRCODE_WARNING_MASK );
/*N*/         }
/*N*/ 
/*N*/ 
/*N*/         xInStrm->closeInput();
/*N*/     }
/*N*/     else
/*?*/         rIn.SetError( ERRCODE_CLASS_READ | ERRCODE_SVX_FORMS_NOIOSERVICES | ERRCODE_WARNING_MASK );
/*N*/ }

//------------------------------------------------------------------------------
/*N*/ void FmFormPageImpl::write(const Reference< ::com::sun::star::io::XObjectOutputStream > & xOutStrm) const
/*N*/ {
/*N*/     Reference< ::com::sun::star::io::XMarkableStream >  xMarkStrm(xOutStrm, UNO_QUERY);
/*N*/     if (!xMarkStrm.is())
/*N*/         return; // exception
/*N*/ 
/*N*/     //  sortieren der objectlist nach der Reihenfolge
/*N*/     FmObjectList aList;
/*N*/     fillList(aList, *pPage, sal_True);
/*N*/ 
/*N*/     // schreiben aller forms
/*N*/     Reference< ::com::sun::star::io::XPersistObject >  xAsPersist(xForms, UNO_QUERY);
/*N*/     if (xAsPersist.is())
/*N*/         xAsPersist->write(xOutStrm);
/*N*/         // don't use the writeObject of the stream, as this wouldn't be compatible with older documents
/*N*/ 
/*N*/     // objectliste einfuegen
/*N*/     sal_Int32 nLength = aList.Count();
/*N*/ 
/*N*/     // schreiben der laenge
/*N*/     xOutStrm->writeLong(nLength);
/*N*/ 
/*N*/     for (sal_Int32 i = 0; i < nLength; i++)
/*N*/     {
/*N*/         // schreiben des Objects mit Marke
/*N*/         // Marke um an den Anfang zu springen
/*N*/         Reference< ::com::sun::star::io::XPersistObject >  xObj(aList.GetObject(i)->GetUnoControlModel(), UNO_QUERY);
/*N*/         if (xObj.is())
/*N*/         {
/*N*/             xOutStrm->writeObject(xObj);
/*N*/         }
/*N*/         else
/*N*/             ;// exception
/*N*/     }
/*N*/ }

//------------------------------------------------------------------------------
/*N*/ void FmFormPageImpl::read(const Reference< ::com::sun::star::io::XObjectInputStream > & xInStrm)
/*N*/ {
/*N*/     Reference< ::com::sun::star::io::XMarkableStream >  xMarkStrm(xInStrm, UNO_QUERY);
/*N*/     if (!xMarkStrm.is())
/*N*/         return; // exception
/*N*/ 
/*N*/     //  sortieren der objectlist nach der Reihenfolge
/*N*/     FmObjectList aList;
/*N*/     fillList(aList, *pPage, sal_False);
/*N*/ 
/*N*/     // lesen aller forms
/*N*/     Reference< ::com::sun::star::io::XPersistObject >  xAsPersist(xForms, UNO_QUERY);
/*N*/     if (xAsPersist.is())
/*N*/         xAsPersist->read(xInStrm);
/*N*/         // don't use the readObject of the stream, as this wouldn't be compatible with older documents
/*N*/ 
/*N*/     // Zuordnung der Formobjekte zu den FormComponents
/*N*/     sal_Int32 nLength = xInStrm->readLong();
/*N*/     DBG_ASSERT(nLength == (sal_Int32) aList.Count(), "Fehler beim Lesen der UnoModels");
/*N*/     for (sal_Int32 i = 0; i < nLength; i++)
/*N*/     {
/*N*/         Reference< ::com::sun::star::awt::XControlModel >  xRef(xInStrm->readObject(), UNO_QUERY);
/*N*/         if (i < (sal_Int32)aList.Count())
/*N*/             aList.GetObject(i)->SetUnoControlModel(xRef);
/*N*/     }
/*N*/ }

//------------------------------------------------------------------------------
/*N*/ void FmFormPageImpl::fillList(FmObjectList& rList, const SdrObjList& rObjList, sal_Bool bConnected) const
/*N*/ {
/*N*/     SdrObjListIter aIter(rObjList);
/*N*/     while (aIter.IsMore())
/*N*/     {
/*N*/         SdrObject* pObj = aIter.Next();
/*N*/         if (pObj && pObj->GetObjInventor() == FmFormInventor)
/*N*/         {
/*N*/             FmFormObj* pFormObj = PTR_CAST(FmFormObj, pObj);
/*N*/             DBG_ASSERT(!bConnected || pFormObj->GetUnoControlModel().is(), "Controlshape ohne Control");
/*N*/             if (!bConnected || pFormObj->GetUnoControlModel().is())
/*N*/                 rList.Insert(pFormObj, LIST_APPEND);
/*N*/ 
/*N*/         }
/*N*/     }
/*N*/ }


}
