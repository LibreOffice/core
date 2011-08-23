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

#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <framework/interaction.hxx>

using namespace ::framework;
namespace binfilter {
 
class FilterOptionsContinuation : public ContinuationBase< ::com::sun::star::document::XInteractionFilterOptions >
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > rProperties;

public:
    virtual void SAL_CALL setFilterOptions( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rProp ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getFilterOptions(  ) throw (::com::sun::star::uno::RuntimeException);
};

class RequestFilterOptions : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    ::com::sun::star::uno::Any m_aRequest;
        
    ::com::sun::star::uno::Sequence< 
                    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > 
                > m_lContinuations;
        
    ContinuationAbort*	m_pAbort;
        
    FilterOptionsContinuation*	m_pOptions;

public:
    RequestFilterOptions( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > rModel,
                              ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > rProperties );
    
    sal_Bool	isAbort() { return m_pAbort->isSelected(); }
        
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > getFilterOptions()
    {
        return m_pOptions->getFilterOptions();
    }
        
    virtual ::com::sun::star::uno::Any SAL_CALL getRequest() 
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< 
                ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > 
            > SAL_CALL getContinuations() 
        throw( ::com::sun::star::uno::RuntimeException );
};  
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
