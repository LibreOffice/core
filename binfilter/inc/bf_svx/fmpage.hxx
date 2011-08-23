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

#ifndef _SVX_FMPAGE_HXX
#define _SVX_FMPAGE_HXX

#ifndef _SVDPAGE_HXX //autogen
#include <bf_svx/svdpage.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
FORWARD_DECLARE_INTERFACE(container,XNameContainer)
namespace binfilter {

class StarBASIC;
class FmFormModel;
class FmFormPageImpl;	// haelt die Liste aller Forms

//FORWARD_DECLARE_INTERFACE(uno,Reference)
//STRIP008 FORWARD_DECLARE_INTERFACE(container,XNameContainer)

class SfxJSArray;
class HelpEvent;

class FmFormPage : public SdrPage
{
    friend class FmFormObj;
    FmFormPageImpl* pImpl;

public:
    TYPEINFO();

    FmFormPage(FmFormModel& rModel,StarBASIC*, FASTBOOL bMasterPage=sal_False);
    ~FmFormPage();

    using SdrPage::NbcInsertObject;
    using SdrPage::ReplaceObject;

    virtual void  	WriteData(SvStream& rOut) const;
    virtual void  	ReadData(const SdrIOHeader& rHead, SvStream& rIn);
    virtual void  	SetModel(SdrModel* pNewModel);

    virtual SdrPage* Clone() const;

    virtual void	InsertObject(SdrObject* pObj, ULONG nPos = CONTAINER_APPEND,
                                    const SdrInsertReason* pReason=NULL);

    virtual SdrObject* RemoveObject(ULONG nObjNum);

#ifndef SVX_LIGHT
    /**	Insert _pClone into the page.
        If _pClone is no form object, InsertObject will be called.
        If _pClone is a form object, the page will create forms as needed to properly place the control model of the new object
        (so it is located within the forms hierarchy in an position analogous to the source objects model with it's hierarchy).
        If _pClone is NULL, it will be created via _pSourceObject->Clone(SdrPage*, SdrModel*).
        If bTryPreserveName is sal_True, the property value for "Name" will be restored after calling the InsertObject (which otherwise
        may alter the name to ensure uniqueness).
    */

    /** Create an environment (within the forms hierarchy of the page) for inserting a form object which is a clone
        of the given form object (which has to belong to a foreign page).
        "environment" means that the UnoControlModel of the given object may be inserted into our forms hierarchy
        without (to much) structure loss.
        If _pObj isn't a form object, nothing happens.
    */
#endif

    // Zugriff auf alle Formulare
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& GetForms() const;

    FmFormPageImpl*  GetImpl() const {return pImpl;}

public:
    String				aPageName;
    StarBASIC* 			pBasic;

    const String& 		GetName() const { return aPageName; }
    void 				SetName( const String& rName ) { aPageName = rName; }
    StarBASIC*      	GetBasic() const { return pBasic; }
};

}//end of namespace binfilter
#endif          // _SVX_FMPAGE_HXX

