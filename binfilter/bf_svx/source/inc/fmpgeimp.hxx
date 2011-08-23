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
FORWARD_DECLARE_INTERFACE(io,XObjectOutputStream)
FORWARD_DECLARE_INTERFACE(io,XObjectInputStream)
FORWARD_DECLARE_INTERFACE(container,XIndexContainer)
class SvStream;
namespace binfilter {

class SdrIOHeader;
class FmFormObj;
class FmFormPage;
class SdrObject;

//FORWARD_DECLARE_INTERFACE(uno,Reference)
//STRIP008 FORWARD_DECLARE_INTERFACE(io,XObjectOutputStream)
//STRIP008 FORWARD_DECLARE_INTERFACE(io,XObjectInputStream)
//STRIP008 FORWARD_DECLARE_INTERFACE(container,XIndexContainer)

class SdrObjList;

DECLARE_LIST(FmObjectList, FmFormObj*)//STRIP008 DECLARE_LIST(FmObjectList, FmFormObj*);

//==================================================================
// FmFormPageImpl
// lauscht an allen Containern, um festzustellen, wann Objecte
// eingefuegt worden sind und wann diese entfernt wurden
//==================================================================

class FmFormPageImpl
{
    friend class FmFormPage;
    friend class FmFormObj;
    friend class FmXFormShell;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>				xCurrentForm;	// aktuelles Formular
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>	xForms;			// Liste aller Forms
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>				xModel;
    FmFormPage*				pPage;
    UniString				m_sPageId;

    sal_Bool		m_bFirstActivation	: 1;

protected:
    FmFormPageImpl(FmFormPage* _pPage);
    FmFormPageImpl(FmFormPage* _pPage, const FmFormPageImpl& rImpl);
    ~FmFormPageImpl();

    void Init();

public:
    //	nur wichtig fuer den DesignMode
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& getCurForm() {return xCurrentForm;}

    // Defaults fuer ein Object setzen
    // Eindeutigen Namen, Zuordnen zu einer Form falls noch nicht erfolgt

    UniString GetPageId() const { return m_sPageId; }

    // activation handling
    inline	sal_Bool	hasEverBeenActivated( ) const { return !m_bFirstActivation; }
    inline	void		setHasBeenActivated( ) { m_bFirstActivation = sal_False; }

protected:
    // lesen und schreiben der Objecte
    void WriteData(SvStream& rOut) const; //
    void ReadData(const SdrIOHeader& rHead, SvStream& rIn); //

    void write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& OutStream) const;
    void read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& InStream);

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& getForms() {return xForms;}

    void fillList(FmObjectList& rList, const SdrObjList& rObjList, sal_Bool bConnected) const;



public:


};


}//end of namespace binfilter
#endif // _SVX_FMUNOPGE_HXX

