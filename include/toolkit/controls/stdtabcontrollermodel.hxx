/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_TOOLKIT_CONTROLS_STDTABCONTROLLERMODEL_HXX
#define INCLUDED_TOOLKIT_CONTROLS_STDTABCONTROLLERMODEL_HXX


#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weakagg.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <osl/mutex.hxx>

#include <tools/gen.hxx>
#include <vector>

struct UnoControlModelEntry;
typedef ::std::vector< UnoControlModelEntry* > UnoControlModelEntryListBase;

class UnoControlModelEntryList
{
private:
    UnoControlModelEntryListBase maList;
    OUString maGroupName;

public:
                    UnoControlModelEntryList();
                    ~UnoControlModelEntryList();

    const OUString&      GetName() const                         { return maGroupName; }
    void                        SetName( const OUString& rName ) { maGroupName = rName; }

    void    Reset();
    void    DestroyEntry( size_t nEntry );
    size_t  size() const;
    UnoControlModelEntry* operator[]( size_t i ) const;
    void push_back( UnoControlModelEntry* item );
    void insert( size_t i, UnoControlModelEntry* item );
};

struct UnoControlModelEntry
{
    bool        bGroup;
    union
    {
        css::uno::Reference< css::awt::XControlModel >* pxControl;
        UnoControlModelEntryList*   pGroup;
    };
};

struct ComponentEntry
{
    css::awt::XWindow*     pComponent;
    Point                               aPos;
};

typedef ::std::vector< ComponentEntry* > ComponentEntryList;

#define CONTROLPOS_NOTFOUND 0xFFFFFFFF

class StdTabControllerModel :   public css::awt::XTabControllerModel,
                                public css::lang::XServiceInfo,
                                public css::io::XPersistObject,
                                public css::lang::XTypeProvider,
                                public ::cppu::OWeakAggObject
{
private:
    ::osl::Mutex                maMutex;
    UnoControlModelEntryList    maControls;
    bool                    mbGroupControl;

protected:
    ::osl::Mutex&           GetMutex() { return maMutex; }
    sal_uInt32              ImplGetControlCount( const UnoControlModelEntryList& rList ) const;
    void                    ImplGetControlModels( css::uno::Reference< css::awt::XControlModel > ** pRefs, const UnoControlModelEntryList& rList ) const;
    static void             ImplSetControlModels( UnoControlModelEntryList& rList, const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Controls );
    static sal_uInt32       ImplGetControlPos( const css::uno::Reference< css::awt::XControlModel >& rCtrl, const UnoControlModelEntryList& rList );

public:
                            StdTabControllerModel();
                            virtual ~StdTabControllerModel();

    // css::uno::XInterface
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return OWeakAggObject::queryInterface(rType); }
    void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }

    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XTabControllerModel
    sal_Bool SAL_CALL getGroupControl(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setGroupControl( sal_Bool GroupControl ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setControlModels( const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Controls ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > SAL_CALL getControlModels(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setGroup( const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group, const OUString& GroupName ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getGroupCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getGroup( sal_Int32 nGroup, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group, OUString& Name ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL getGroupByName( const OUString& Name, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group ) throw(css::uno::RuntimeException, std::exception) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL write( const css::uno::Reference< css::io::XObjectOutputStream >& OutStream ) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
    void SAL_CALL read( const css::uno::Reference< css::io::XObjectInputStream >& InStream ) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};



#endif // _TOOLKIT_HELPER_STDTABCONTROLLERMODEL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
