/*************************************************************************
 *
 *  $RCSfile: Edit.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:05 $
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

#ifndef _FORMS_EDIT_HXX_
#define _FORMS_EDIT_HXX_

#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
#endif

//.........................................................................
namespace frm
{

//==================================================================
//= OEditModel
//==================================================================
class OEditModel
                :public OEditBaseModel
                ,public ::utl::OAggregationArrayUsageHelper< OEditModel >
{
    staruno::Reference<starutil::XNumberFormatter>      m_xFormatter;
    ::rtl::OUString                 m_aSaveValue;
    sal_Int32                   m_nFormatKey;
    Date                    m_aNullDate;
    sal_Int32                   m_nFieldType;
    sal_Int16                   m_nKeyType;
    sal_Int16                   m_nMaxLen;

    sal_Bool    m_bWritingFormattedFake;
        // are we writing something which should be interpreted as formatted upon reading

    static sal_Int32            nTextHandle;

protected:
    virtual void _onValueChanged();
    virtual staruno::Sequence<staruno::Type> _getTypes();

    OEditModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    virtual ~OEditModel();

    void enableFormattedWriteFake() { m_bWritingFormattedFake = sal_True; }
    void disableFormattedWriteFake() { m_bWritingFormattedFake = sal_False; }
    sal_Bool lastReadWasFormattedFake() const { return (getLastReadVersion() & PF_FAKE_FORMATTED_FIELD) != 0; }

    friend InterfaceRef SAL_CALL OEditModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    friend class OFormattedFieldWrapper;
    friend class OFormattedModel;   // temporary

public:
    virtual void SAL_CALL disposing();

// starform::XBoundComponent
    virtual sal_Bool _commit();

// stario::XPersistObject
    virtual void SAL_CALL write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream);
    virtual ::rtl::OUString SAL_CALL getServiceName();

// starbeans::XPropertySet
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// starlang::XServiceInfo
    IMPLEMENTATION_NAME(OEditModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// starform::XLoadListener
    virtual void _loaded(const starlang::EventObject& rEvent);
    virtual void _unloaded();

// starform::XReset
    virtual void _reset();

// OAggregationArrayUsageHelper
    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    virtual sal_Int16 getPersistenceFlags() const;
};

//==================================================================
//= OEditControl
//==================================================================
class OEditControl : public OBoundControl
                      ,public starawt::XFocusListener
                      ,public starawt::XKeyListener
                      ,public starform::XChangeBroadcaster
{
    ::cppu::OInterfaceContainerHelper
                        m_aChangeListeners;

    ::rtl::OUString         m_aHtmlChangeValue;
    sal_uInt32              m_nKeyEvent;

public:
    OEditControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    virtual ~OEditControl();

    DECLARE_UNO3_AGG_DEFAULTS(OEditControl, OBoundControl);
    virtual staruno::Any SAL_CALL queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException);

    virtual staruno::Sequence<staruno::Type> _getTypes();

// OComponentHelper
    virtual void SAL_CALL disposing();

// starlang::XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& _rSource) throw(staruno::RuntimeException);

// starlang::XServiceInfo
    IMPLEMENTATION_NAME(OEditControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// starform::XChangeBroadcaster
    virtual void SAL_CALL addChangeListener(const staruno::Reference<starform::XChangeListener>& _rxListener);
    virtual void SAL_CALL removeChangeListener(const staruno::Reference<starform::XChangeListener>& _rxListener);

// starawt::XFocusListener
    virtual void SAL_CALL focusGained( const starawt::FocusEvent& e );
    virtual void SAL_CALL focusLost( const starawt::FocusEvent& e );

// starawt::XKeyListener
    virtual void SAL_CALL keyPressed(const starawt::KeyEvent& e);
    virtual void SAL_CALL keyReleased(const starawt::KeyEvent& e);

private:
    DECL_LINK( OnKeyPressed, void* );
};

//.........................................................................
}
//.........................................................................

#endif // _FORMS_EDIT_HXX_

