/*************************************************************************
 *
 *  $RCSfile: UnoNameItemTable.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-29 12:37:04 $
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

#ifndef _SVX_UNONAMEITEMTABLE_HXX_
#define _SVX_UNONAMEITEMTABLE_HXX_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#include <cppuhelper/implbase2.hxx>

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#ifndef _SVX_XIT_HXX
#include "xit.hxx"
#endif

#include "xdef.hxx"

class SdrModel;
class SfxItemPool;
class SfxItemSet;

typedef std::vector< std::pair< SfxItemSet*, SfxItemSet*> > ItemPoolVector;
class SvxUnoNameItemTable : public cppu::WeakImplHelper2< com::sun::star::container::XNameContainer, com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    SdrModel*       mpModel;
    SfxItemPool*    mpModelPool;
    SfxItemPool*    mpStylePool;
    USHORT          mnWhich;
    BYTE            mnMemberId;

    ItemPoolVector maItemSetVector;

public:
    SvxUnoNameItemTable( SdrModel* pModel, USHORT nWhich, BYTE nMemberId = 0 ) throw();
    virtual ~SvxUnoNameItemTable() throw();

    virtual NameOrIndex* createItem() const throw() = 0;
    void dispose();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw ();

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const  rtl::OUString& ServiceName ) throw( com::sun::star::uno::RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const  rtl::OUString& aName, const  com::sun::star::uno::Any& aElement ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::container::ElementExistException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  rtl::OUString& Name ) throw( com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  rtl::OUString& aName, const  com::sun::star::uno::Any& aElement ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual com::sun::star::uno::Any SAL_CALL getByName( const  rtl::OUString& aName ) throw( com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence<  rtl::OUString > SAL_CALL getElementNames(  ) throw( com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const  rtl::OUString& aName ) throw( com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) throw( com::sun::star::uno::RuntimeException);
};

#endif // _SVX_UNONAMEITEMTABLE_HXX_
