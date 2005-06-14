/*************************************************************************
 *
 *  $RCSfile: fmpgeimp.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2005-06-14 16:33:47 $
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
#ifndef _SVX_FMUNOPGE_HXX
#define _SVX_FMUNOPGE_HXX

#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif


#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/dllapi.h"
#endif

class SvStream;
//BFS01class SdrIOHeader;
class FmFormObj;
class FmFormPage;
class SdrObject;

//FORWARD_DECLARE_INTERFACE(uno,Reference)
FORWARD_DECLARE_INTERFACE(io,XObjectOutputStream)
FORWARD_DECLARE_INTERFACE(io,XObjectInputStream)
FORWARD_DECLARE_INTERFACE(container,XIndexContainer)

class SdrObjList;

DECLARE_LIST(FmObjectList, FmFormObj*);

//==================================================================
// FmFormPageImpl
// lauscht an allen Containern, um festzustellen, wann Objecte
// eingefuegt worden sind und wann diese entfernt wurden
//==================================================================

class SVX_DLLPRIVATE FmFormPageImpl
{
    friend class FmFormPage;
    friend class FmFormObj;
    friend class FmXFormShell;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>                xCurrentForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>  m_xForms;
    FmFormPage*             pPage;

    sal_Bool        m_bFirstActivation  : 1;
    sal_Bool        m_bAttemptedFormCreation : 2;

protected:
    FmFormPageImpl(FmFormPage* _pPage);
    FmFormPageImpl(FmFormPage* _pPage, const FmFormPageImpl& rImpl);
    ~FmFormPageImpl();

    void Init();

public:
    //  nur wichtig fuer den DesignMode
    void setCurForm(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm);
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> getDefaultForm();

    /** inserts a form component into the form component hierarchy

        If the given component does not yet belong into the form hierarchy, a suitable place for
        it is found, using <member>findFormForDataSource</member>, if possible.

        If no sutiable form is found, a new one is created, and also inserted into the
        hierarchy.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> placeInFormComponentHierarchy(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent>& rContent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& rDatabase = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>(),
        const ::rtl::OUString& rDBTitle = ::rtl::OUString(),
        const ::rtl::OUString& rCursorSource = ::rtl::OUString(),
        sal_Int32 nCommandType = 0
    );

    // activation handling
    inline  sal_Bool    hasEverBeenActivated( ) const { return !m_bFirstActivation; }
    inline  void        setHasBeenActivated( ) { m_bFirstActivation = sal_False; }

protected:
    // lesen und schreiben der Objecte
//BFS01 void WriteData(SvStream& rOut) const; //
//BFS01 void ReadData(const SdrIOHeader& rHead, SvStream& rIn); //

    void write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& OutStream) const;
    void read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& InStream);

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& getForms( bool _bForceCreate = true );

    void fillList(FmObjectList& rList, const SdrObjList& rObjList, sal_Bool bConnected) const;

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
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> findFormForDataSource(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& rForm,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& rDatabase,
        const ::rtl::OUString& rCommand,
        sal_Int32 nCommandType
    );

    ::rtl::OUString getDefaultName(
                        sal_Int16 _nClassId,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& _rxControls,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo >& _rxObject
                    ) const;

private:
    /** validates whether <member>xCurrentForm</member> is still valid and to be used

        There are situations where our current form becomes invalid, without us noticing this. Thus,
        every method which accesses <member>xCurrentForm</member> should beforehand validate the current
        form by calling this method.

        If <member>xCurrentForm</member> is not valid anymore, it is reset to <NULL/>.

        @since #i40086#
    */
    void    validateCurForm();

public:

    static UniString getDefaultName(
                        sal_Int16 nClassId,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo >& _rxObject
                    );

    ::rtl::OUString setUniqueName(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent>& xFormComponent, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& xControls);
    ::rtl::OUString getUniqueName(const ::rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& xNamedSet) const;
};


#endif // _SVX_FMUNOPGE_HXX

