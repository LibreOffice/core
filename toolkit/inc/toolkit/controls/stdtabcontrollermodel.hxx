/*************************************************************************
 *
 *  $RCSfile: stdtabcontrollermodel.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:08 $
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

#ifndef _TOOLKIT_CONTROLS_STDTABCONTROLLERMODEL_HXX_
#define _TOOLKIT_CONTROLS_STDTABCONTROLLERMODEL_HXX_


#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <tools/list.hxx>
#include <tools/gen.hxx>

struct UnoControlModelEntry;

DECLARE_LIST( UnoControlModelEntryListBase, UnoControlModelEntry* );

class UnoControlModelEntryList : public UnoControlModelEntryListBase
{
private:
    ::rtl::OUString maGroupName;

public:
                    UnoControlModelEntryList();
                    ~UnoControlModelEntryList();

    const ::rtl::OUString&      GetName() const                         { return maGroupName; }
    void                        SetName( const ::rtl::OUString& rName ) { maGroupName = rName; }

    void    Reset();
    void    DestroyEntry( sal_uInt32 nEntry );
};

struct UnoControlModelEntry
{
    sal_Bool        bGroup;
    union
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >* pxControl;
        UnoControlModelEntryList*   pGroup;
    };
};

// Keine Referenz halten, nur temporaer fuer AutoTabOrder
struct ComponentEntry
{
    ::com::sun::star::awt::XWindow*     pComponent;
    Point                               aPos;
};

DECLARE_LIST( ComponentEntryList, ComponentEntry* );

#define CONTROLPOS_NOTFOUND 0xFFFFFFFF

class StdTabControllerModel :   public ::com::sun::star::awt::XTabControllerModel,
                                public ::com::sun::star::io::XPersistObject,
                                public ::com::sun::star::lang::XTypeProvider,
                                public ::cppu::OWeakAggObject
{
private:
    ::osl::Mutex                maMutex;
    UnoControlModelEntryList    maControls;
    sal_Bool                    mbGroupControl;

protected:
    ::osl::Mutex&           GetMutex() { return maMutex; }
    sal_uInt32              ImplGetControlCount( const UnoControlModelEntryList& rList ) const;
    void                    ImplGetControlModels( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > ** pRefs, const UnoControlModelEntryList& rList ) const;
    void                    ImplSetControlModels( UnoControlModelEntryList& rList, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) const;
    sal_uInt32              ImplGetControlPos( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xCtrl, const UnoControlModelEntryList& rList ) const;

public:
                            StdTabControllerModel();
                            ~StdTabControllerModel();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return OWeakAggObject::queryInterface(rType); }
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTabControllerModel
    sal_Bool SAL_CALL getGroupControl(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setGroupControl( sal_Bool GroupControl ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setControlModels( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > SAL_CALL getControlModels(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, const ::rtl::OUString& GroupName ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getGroupCount(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getGroup( sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, ::rtl::OUString& Name ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getGroupByName( const ::rtl::OUString& Name, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};



#endif // _TOOLKIT_HELPER_STDTABCONTROLLERMODEL_HXX_

