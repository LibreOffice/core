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

/** this file implements an export of a selected EditEngine content into
    a xml stream. See bf_svx/source/inc/xmledit.hxx for interface */

#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_ 
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_ 
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif 











#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif


#ifndef _XMLOFF_XMLEXP_HXX
#include "bf_xmloff/xmlexp.hxx"
#endif


#ifndef _SVX_UNOFORED_HXX 
#include "unofored.hxx"
#endif

#ifndef _SVX_UNOTEXT_HXX 
#include "unotext.hxx"
#endif

#ifndef _SVX_EDITSOURCE_HXX
#include "editsource.hxx"
#endif

#include <cppuhelper/implbase4.hxx>

#ifndef _SVX_UNOFIELD_HXX 
#include "unofield.hxx"
#endif

#ifndef SVX_UNOMOD_HXX 
#include "unomod.hxx"
#endif
#include "unonrule.hxx"
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::rtl;
using namespace cppu;

///////////////////////////////////////////////////////////////////////

class SvxEditEngineSourceImpl;

///////////////////////////////////////////////////////////////////////

/*N*/ class SvxEditEngineSourceImpl
/*N*/ {
/*N*/ private:
/*N*/ 	oslInterlockedCount	maRefCount;
/*N*/ 
/*N*/ 	EditEngine*				mpEditEngine;
/*N*/ 	SvxTextForwarder*		mpTextForwarder;
/*N*/ 
/*N*/ 	~SvxEditEngineSourceImpl();
/*N*/ 
/*N*/ public:
/*N*/ 	void SAL_CALL acquire();
/*N*/ 	void SAL_CALL release();
/*N*/ 
/*N*/ 	SvxTextForwarder*		GetTextForwarder();
/*N*/ };
/*N*/ 
/*N*/ ///////////////////////////////////////////////////////////////////////
/*N*/ 
/*N*/ SvxEditEngineSourceImpl::~SvxEditEngineSourceImpl()
/*N*/ {
/*N*/ 	delete mpTextForwarder;
/*N*/ }
/*N*/ 
/*N*/ //------------------------------------------------------------------------
/*N*/ 
/*N*/ void SAL_CALL SvxEditEngineSourceImpl::acquire() 
/*N*/ {
/*N*/ 	osl_incrementInterlockedCount( &maRefCount );
/*N*/ }	
/*N*/ 
/*N*/ //------------------------------------------------------------------------
/*N*/ 
/*N*/ void SAL_CALL SvxEditEngineSourceImpl::release()
/*N*/ {
/*N*/ 	if( ! osl_decrementInterlockedCount( &maRefCount ) )
/*N*/ 		delete this;
/*N*/ }	
/*N*/ 
/*N*/ //------------------------------------------------------------------------
/*N*/ 
/*N*/ SvxTextForwarder* SvxEditEngineSourceImpl::GetTextForwarder()
/*N*/ {
/*N*/ 	if (!mpTextForwarder)
/*N*/ 		mpTextForwarder = new SvxEditEngineForwarder( *mpEditEngine );
/*N*/ 
/*N*/ 	return mpTextForwarder;
/*N*/ }
/*N*/ 
/*N*/ // --------------------------------------------------------------------
/*N*/ // SvxTextEditSource
/*N*/ // --------------------------------------------------------------------
/*N*/ 
/*N*/ SvxEditEngineSource::SvxEditEngineSource( SvxEditEngineSourceImpl* pImpl )
/*N*/ {
/*N*/ 	mpImpl = pImpl;
/*N*/ 	mpImpl->acquire();
/*N*/ }
/*N*/ 
/*N*/ //------------------------------------------------------------------------
/*N*/ 
/*N*/ SvxEditEngineSource::~SvxEditEngineSource()
/*N*/ {
/*N*/ 	mpImpl->release();
/*N*/ }
/*N*/ 
/*N*/ //------------------------------------------------------------------------
/*N*/ 
/*N*/ SvxEditSource* SvxEditEngineSource::Clone() const
/*N*/ {
/*N*/ 	return new SvxEditEngineSource( mpImpl );
/*N*/ }
/*N*/ 
/*N*/ //------------------------------------------------------------------------
/*N*/ 
/*N*/ SvxTextForwarder* SvxEditEngineSource::GetTextForwarder()
/*N*/ {
/*N*/     return mpImpl->GetTextForwarder();
/*N*/ }
/*N*/ 
/*N*/ //------------------------------------------------------------------------
/*N*/ 
/*N*/ void SvxEditEngineSource::UpdateData()
/*N*/ {
/*N*/ }
}
