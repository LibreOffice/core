/*************************************************************************
 *
 *  $RCSfile: dlgedobj.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: tbe $ $Date: 2001-03-12 11:30:48 $
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

#ifndef _BASCTL_DLGEDOBJ_HXX
#define _BASCTL_DLGEDOBJ_HXX

#ifndef _SVDOUNO_HXX
#include <svx/svdouno.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif

class DlgEdForm;

//============================================================================
// DlgEdObj
//============================================================================

class DlgEdObj: public SdrUnoObj
{
    friend class VCDlgEditor;
    friend class VCDlgEditFactory;
    friend class DlgEdPropListenerImpl;

private:
    sal_Bool        bIsListening;
    sal_uInt32      nEvent;
    DlgEdForm*      pDlgEdForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener> m_xPropertyChangeListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener> m_xContainerListener;

public:
    TYPEINFO();

protected:
    DlgEdObj();
    DlgEdObj(const ::rtl::OUString& rModelName);
    DlgEdObj(const ::rtl::OUString& rModelName,
             const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac);

public:
    virtual ~DlgEdObj();
    virtual void SetPage(SdrPage* pNewPage);

    virtual void SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    virtual DlgEdForm* GetDlgEdForm() const { return pDlgEdForm; }

    virtual void SetRectFromProps();
    virtual void SetPropsFromRect();
    virtual void SAL_CALL SetNameFromProp( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException);

    virtual sal_Int32   GetStep() const;
    virtual void        SetStep( sal_Int32 nStep );
    virtual void        UpdateStep();

    String  GetUniqueName();

    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;

    virtual SdrObject*  Clone() const;
    virtual SdrObject*  Clone(SdrPage* pPage, SdrModel* pModel) const;
    virtual void        operator= (const SdrObject& rObj);

    virtual void clonedFrom(const DlgEdObj* _pSource);

protected:
    virtual void     WriteData(SvStream& rOut) const;
    virtual void     ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    virtual void NbcMove( const Size& rSize );
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

    DECL_LINK(OnCreate, void* );

    String  GetDefaultName();

public:
    // PropertyChangeListener
    virtual void SAL_CALL _propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

    // ContainerListener
    virtual void SAL_CALL _elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL _elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL _elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

private:
    // start listening
    void StartListening();
    // end listening
    void EndListening(sal_Bool bRemoveListener = sal_True);
    sal_Bool    isListening() const { return bIsListening; }
};


//============================================================================
// DlgEdForm
//============================================================================

class DlgEdForm: public DlgEdObj
{
    friend class VCDlgEditor;
    friend class VCDlgEditFactory;

private:
    VCDlgEditor* pDlgEditor;

public:
    TYPEINFO();

protected:
    DlgEdForm(const ::rtl::OUString& rModelName);
    DlgEdForm(const ::rtl::OUString& rModelName,
              const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac);
    DlgEdForm();

public:
    virtual ~DlgEdForm();
    virtual void SetDlgEditor( VCDlgEditor* pEditor ) { pDlgEditor = pEditor; }
    virtual VCDlgEditor* GetDlgEditor() const { return pDlgEditor; }

    virtual void UpdateStep();

    virtual SdrObject* CheckHit(const Point& rPnt,USHORT nTol,const SetOfByte*) const;

protected:
    virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

};

#endif // _BASCTL_DLGEDOBJ_HXX

