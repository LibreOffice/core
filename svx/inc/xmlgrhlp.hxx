/*************************************************************************
 *
 *  $RCSfile: xmlgrhlp.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: ka $
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

#ifndef _XMLGRHLP_HXX
#define _XMLGRHLP_HXX

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif
#ifndef __SGI_STL_SET
#include <stl/set>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

// ----------
// - SvXMLGraphicHelper

enum SvXMLGraphicHelperMode
{
    GRAPHICHELPER_MODE_READ = 0,
    GRAPHICHELPER_MODE_WRITE = 1
};

// ----------------------
// - SvXMLGraphicHelper -
// ----------------------

class SvStorage;

class SvXMLGraphicHelper : public ::cppu::WeakComponentImplHelper1< ::com::sun::star::container::XIndexContainer >
{
private:


    SvStorage*                          mpRootStorage;
    SvStorageRef                        mxGraphicStorage;
    ::osl::Mutex                        maMutex;
    ::_STL::vector< ::rtl::OUString >   maVector;
    ::_STL::set< ::rtl::OUString >      maSet;
    SvXMLGraphicHelperMode              meCreateMode;
    BOOL                                mbDirect;

    SvStorageRef                        ImplGetGraphicStorage();
    SvStorageStreamRef                  ImplGetGraphicStream( const ::rtl::OUString& rURLStr, BOOL bTruncate );
    void                                ImplWriteGraphic( const ::rtl::OUString& rURLStr );
    void                                ImplInsertGraphicURL( const ::rtl::OUString& rURLStr );

protected:


                                        SvXMLGraphicHelper();
                                        ~SvXMLGraphicHelper();
    void                                Init( SvStorage& rXMLStorage,
                                              SvXMLGraphicHelperMode eCreateMode,
                                              BOOL bDirect );

public:

    static SvXMLGraphicHelper*          Create( SvStorage& rXMLStorage,
                                                SvXMLGraphicHelperMode eCreateMode,
                                                BOOL bDirect = TRUE );
    static void                         Destroy( SvXMLGraphicHelper* pSvXMLGraphicHelper );

    void                                Flush();

public:

    // XIndexContainer
    virtual void SAL_CALL                           insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
                                                        throw(  ::com::sun::star::lang::IllegalArgumentException,
                                                                ::com::sun::star::lang::IndexOutOfBoundsException,
                                                                ::com::sun::star::lang::WrappedTargetException,
                                                                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL                           removeByIndex( sal_Int32 Index )
                                                        throw(  ::com::sun::star::lang::IndexOutOfBoundsException,
                                                                ::com::sun::star::lang::WrappedTargetException,
                                                                ::com::sun::star::uno::RuntimeException );

    // XIndexReplace
    virtual void SAL_CALL                           replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
                                                        throw(  ::com::sun::star::lang::IllegalArgumentException,
                                                                ::com::sun::star::lang::IndexOutOfBoundsException,
                                                                ::com::sun::star::lang::WrappedTargetException,
                                                                ::com::sun::star::uno::RuntimeException );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL                      getCount()
                                                        throw(  ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL     getByIndex( sal_Int32 Index )
                                                        throw(  ::com::sun::star::lang::IndexOutOfBoundsException,
                                                                ::com::sun::star::lang::WrappedTargetException,
                                                                ::com::sun::star::uno::RuntimeException );

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL    getElementType()
                                                        throw(  ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL                       hasElements()
                                                        throw(  ::com::sun::star::uno::RuntimeException );
};

#endif
