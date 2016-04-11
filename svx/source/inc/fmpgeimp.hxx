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
#ifndef INCLUDED_SVX_SOURCE_INC_FMPGEIMP_HXX
#define INCLUDED_SVX_SOURCE_INC_FMPGEIMP_HXX

#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XForms.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XMap.hpp>

#include <tools/link.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/weakref.hxx>

#include "svx/svxdllapi.h"
#include <map>

class FmFormObj;
class FmFormPage;
class SdrObject;


// FmFormPageImpl
// lauscht an allen Containern, um festzustellen, wann Objecte
// eingefuegt worden sind und wann diese entfernt wurden


class SVX_DLLPRIVATE FmFormPageImpl
{
    ::std::map< css::uno::Reference< css::form::XFormComponent >,SdrObject* > m_aComponentMap;
    css::uno::Reference< css::form::XForm >               xCurrentForm;
    css::uno::Reference< css::form::XForms >              m_xForms;
    css::uno::WeakReference< css::container::XMap >       m_aControlShapeMap;

    FmFormPage&     m_rPage;
    Link<FmFormPageImpl&,void>  m_aFormsCreationHdl;

    bool            m_bFirstActivation;
    bool            m_bAttemptedFormCreation;

public:
    explicit FmFormPageImpl( FmFormPage& _rPage );
    ~FmFormPageImpl();

    void initFrom( FmFormPageImpl& i_foreignImpl );

    //  nur wichtig fuer den DesignMode
    void setCurForm(const css::uno::Reference< css::form::XForm>& xForm);
    css::uno::Reference< css::form::XForm> getDefaultForm();

    /** finds a place in the form component hierarchy where to insert the given component

        Note that no actual insertion happens, this is the responsibility of the caller (as
        the caller might decide on a suitable place where in the returned container the insertion
        should happen).
    */
    css::uno::Reference< css::form::XForm> findPlaceInFormComponentHierarchy(
        const css::uno::Reference< css::form::XFormComponent>& rContent,
        const css::uno::Reference< css::sdbc::XDataSource>& rDatabase = css::uno::Reference< css::sdbc::XDataSource>(),
        const OUString& rDBTitle = OUString(),
        const OUString& rCursorSource = OUString(),
        sal_Int32 nCommandType = 0
    );

    // activation handling
    inline  bool    hasEverBeenActivated( ) const { return !m_bFirstActivation; }
    inline  void        setHasBeenActivated( ) { m_bFirstActivation = false; }

    const css::uno::Reference< css::form::XForms>& getForms( bool _bForceCreate = true );

    void        SetFormsCreationHdl( const Link<FmFormPageImpl&,void>& _rFormsCreationHdl ) { m_aFormsCreationHdl = _rFormsCreationHdl; }

protected:
    /** finds a form with a given data source signature
        @param rForm
            the form to start the search with. This form, including all possible sub forms,
            will be examined
        @param rDatabase
            the data source which to which the found form must be bound
        @param rCommand
            the desired Command property value of the sought-after form
        @param nCommandType
            the desired CommandType property value of the sought-after form
    */
    css::uno::Reference< css::form::XForm> findFormForDataSource(
        const css::uno::Reference< css::form::XForm>& rForm,
        const css::uno::Reference< css::sdbc::XDataSource>& rDatabase,
        const OUString& rCommand,
        sal_Int32 nCommandType
    );

public:
    static OUString setUniqueName(const css::uno::Reference< css::form::XFormComponent>& xFormComponent, const css::uno::Reference< css::form::XForm>& xControls);

    void formObjectInserted( const FmFormObj& _object );
    void formObjectRemoved( const FmFormObj& _object );
    void formModelAssigned( const FmFormObj& _object );

    /** returns an object mapping from control models to drawing shapes.
    */
    SVX_DLLPUBLIC css::uno::Reference< css::container::XMap > getControlToShapeMap();

private:
    /** validates whether <member>xCurrentForm</member> is still valid and to be used

        There are situations where our current form becomes invalid, without us noticing this. Thus,
        every method which accesses <member>xCurrentForm</member> should beforehand validate the current
        form by calling this method.

        If <member>xCurrentForm</member> is not valid anymore, it is reset to <NULL/>.

        @return
            <TRUE/> if and only if xCurrentForm is valid.

        @since #i40086#
    */
    bool    validateCurForm();

    css::uno::Reference< css::container::XMap >
        impl_createControlShapeMap_nothrow();

private:
    FmFormPageImpl( const FmFormPageImpl& ) = delete;
    FmFormPageImpl& operator=( const FmFormPageImpl& ) = delete;
};


#endif // INCLUDED_SVX_SOURCE_INC_FMPGEIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
