/*************************************************************************
 *
 *  $RCSfile: unostorageholder.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:53:11 $
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

#ifndef _UNOSTORAGEHOLDER_HXX
#define _UNOSTORAGEHOLDER_HXX

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONLISTENER_HPP_
#include <com/sun/star/embed/XTransactionListener.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <unotools/tempfile.hxx>
#include "storage.hxx"

class SotStorage;
class UNOStorageHolder : public ::cppu::WeakImplHelper1<
                          ::com::sun::star::embed::XTransactionListener >

{
    SotStorage* m_pParentStorage;                   // parent storage
    SotStorageRef m_rSotStorage;                    // original substorage
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xStorage;   // duplicate storage
    ::utl::TempFile* m_pTempFile;                   // temporary file used by duplicate storage

public:
    UNOStorageHolder( SotStorage& aParentStorage,
                      SotStorage& aStorage,
                      ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > xStorage,
                      ::utl::TempFile* pTempFile );

    void InternalDispose();
    String GetStorageName();

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetDuplicateStorage() { return m_xStorage; }

    virtual void SAL_CALL preCommit( const ::com::sun::star::lang::EventObject& aEvent )
        throw ( ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL commited( const ::com::sun::star::lang::EventObject& aEvent )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL preRevert( const ::com::sun::star::lang::EventObject& aEvent )
        throw ( ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL reverted( const ::com::sun::star::lang::EventObject& aEvent )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw ( ::com::sun::star::uno::RuntimeException );
};

#endif

