/*************************************************************************
 *
 *  $RCSfile: Button.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:04 $
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

#ifndef _FRM_BUTTON_HXX_
#define _FRM_BUTTON_HXX_

#ifndef _FRM_IMAGE_HXX_
#include "Image.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_ACTIONEVENT_HPP_
#include <com/sun/star/awt/ActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XACTIONLISTENER_HPP_
#include <com/sun/star/awt/XActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBUTTON_HPP_
#include <com/sun/star/awt/XButton.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OButtonModel
//==================================================================
class OButtonModel
        :public OImageModel
        ,public ::utl::OAggregationArrayUsageHelper<OButtonModel>
{
public:
    OButtonModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    ~OButtonModel();

// starlang::XServiceInfo
    IMPLEMENTATION_NAME(OButtonModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// staruno::Reference<starbeans::XPropertySet>
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// stario::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName();
    virtual void SAL_CALL write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream);

    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()
};

//==================================================================
// OButtonControl
//==================================================================
class OButtonControl:  public starawt::XButton,
                         public starawt::XActionListener,
                         public OImageControl
{
    sal_uInt32 nClickEvent;

protected:

    // UNO Anbindung
    virtual staruno::Sequence<staruno::Type> _getTypes();

public:
    OButtonControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    virtual ~OButtonControl();

// starlang::XServiceInfo
    IMPLEMENTATION_NAME(OButtonControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OButtonControl, OImageControl);
    virtual staruno::Any SAL_CALL queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException);

// starawt::XActionListener
    virtual void SAL_CALL actionPerformed(const starawt::ActionEvent& rEvent);

// starawt::XButton
    virtual void SAL_CALL addActionListener(const staruno::Reference<starawt::XActionListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeActionListener(const staruno::Reference<starawt::XActionListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL setLabel(const ::rtl::OUString& Label) throw(staruno::RuntimeException);
    virtual void SAL_CALL setActionCommand(const ::rtl::OUString& _rCommand) throw(staruno::RuntimeException);

// starlang::XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& _rSource) throw(staruno::RuntimeException)
        { OControl::disposing(_rSource); }
private:
    DECL_LINK( OnClick, void* );
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_BUTTON_HXX_

