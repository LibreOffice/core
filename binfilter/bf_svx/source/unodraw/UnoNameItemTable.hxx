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

#ifndef _SVX_UNONAMEITEMTABLE_HXX_
#define _SVX_UNONAMEITEMTABLE_HXX_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#include <cppuhelper/implbase2.hxx>

#include <vector>

#ifndef _SFXLSTNER_HXX 
#include <bf_svtools/lstner.hxx>
#endif

#ifndef _SVX_XIT_HXX
#include "xit.hxx"
#endif

namespace binfilter {

class SfxItemPool;
class SfxItemSet;
class SdrModel;

typedef std::vector< SfxItemSet* > ItemPoolVector;
class SvxUnoNameItemTable : public cppu::WeakImplHelper2< ::com::sun::star::container::XNameContainer, ::com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    SdrModel*		mpModel;
    SfxItemPool*	mpModelPool;
    USHORT			mnWhich;
    BYTE			mnMemberId;

    ItemPoolVector maItemSetVector;

    void SAL_CALL ImplInsertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement );

public:
    SvxUnoNameItemTable( SdrModel* pModel, USHORT nWhich, BYTE nMemberId = 0 ) throw();
    virtual	~SvxUnoNameItemTable() throw();

    virtual NameOrIndex* createItem() const throw() = 0;
    void dispose();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw ();

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const  ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const  ::rtl::OUString& aName, const  ::com::sun::star::uno::Any& aElement ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  ::rtl::OUString& Name ) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  ::rtl::OUString& aName, const  ::com::sun::star::uno::Any& aElement ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const  ::rtl::OUString& aName ) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<  ::rtl::OUString > SAL_CALL getElementNames(  ) throw( ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const  ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) throw( ::com::sun::star::uno::RuntimeException);
};

}//end of namespace binfilter
#endif // _SVX_UNONAMEITEMTABLE_HXX_
