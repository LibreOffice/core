/*************************************************************************
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

#ifndef TOOLKIT_STYLESETTINGS_HXX
#define TOOLKIT_STYLESETTINGS_HXX

/** === begin UNO includes === **/
#include <com/sun/star/awt/XStyleSettings.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <boost/scoped_ptr.hpp>

namespace vos
{
    class IMutex;
}
namespace osl
{
    class Mutex;
}

class VCLXWindow;

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    //==================================================================================================================
    //= WindowStyleSettings
    //==================================================================================================================
    struct WindowStyleSettings_Data;
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::awt::XStyleSettings
                                    >   WindowStyleSettings_Base;
    class WindowStyleSettings : public WindowStyleSettings_Base
    {
    public:
        WindowStyleSettings( ::vos::IMutex& i_rWindowMutex, ::osl::Mutex& i_rListenerMutex, VCLXWindow& i_rOwningWindow );
        ~WindowStyleSettings();

        void dispose();

        // XStyleSettings
        virtual ::sal_Int32 SAL_CALL getActiveBorderColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setActiveBorderColor( ::sal_Int32 _activebordercolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getActiveColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setActiveColor( ::sal_Int32 _activecolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getActiveTabColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setActiveTabColor( ::sal_Int32 _activetabcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getActiveTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setActiveTextColor( ::sal_Int32 _activetextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getButtonRolloverTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setButtonRolloverTextColor( ::sal_Int32 _buttonrollovertextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getButtonTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setButtonTextColor( ::sal_Int32 _buttontextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getCheckedColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCheckedColor( ::sal_Int32 _checkedcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getDarkShadowColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDarkShadowColor( ::sal_Int32 _darkshadowcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getDeactiveBorderColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDeactiveBorderColor( ::sal_Int32 _deactivebordercolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getDeactiveColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDeactiveColor( ::sal_Int32 _deactivecolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getDeactiveTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDeactiveTextColor( ::sal_Int32 _deactivetextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getDialogColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDialogColor( ::sal_Int32 _dialogcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getDialogTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDialogTextColor( ::sal_Int32 _dialogtextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getDisableColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDisableColor( ::sal_Int32 _disablecolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getFaceColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFaceColor( ::sal_Int32 _facecolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getFaceGradientColor() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getFieldColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFieldColor( ::sal_Int32 _fieldcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getFieldRolloverTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFieldRolloverTextColor( ::sal_Int32 _fieldrollovertextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getFieldTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFieldTextColor( ::sal_Int32 _fieldtextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getGroupTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setGroupTextColor( ::sal_Int32 _grouptextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getHelpColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHelpColor( ::sal_Int32 _helpcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getHelpTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHelpTextColor( ::sal_Int32 _helptextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getHighlightColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHighlightColor( ::sal_Int32 _highlightcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getHighlightTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHighlightTextColor( ::sal_Int32 _highlighttextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getInactiveTabColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setInactiveTabColor( ::sal_Int32 _inactivetabcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getInfoTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setInfoTextColor( ::sal_Int32 _infotextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getLabelTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLabelTextColor( ::sal_Int32 _labeltextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getLightColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLightColor( ::sal_Int32 _lightcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMenuBarColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMenuBarColor( ::sal_Int32 _menubarcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMenuBarTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMenuBarTextColor( ::sal_Int32 _menubartextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMenuBorderColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMenuBorderColor( ::sal_Int32 _menubordercolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMenuColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMenuColor( ::sal_Int32 _menucolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMenuHighlightColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMenuHighlightColor( ::sal_Int32 _menuhighlightcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMenuHighlightTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMenuHighlightTextColor( ::sal_Int32 _menuhighlighttextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMenuTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMenuTextColor( ::sal_Int32 _menutextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMonoColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMonoColor( ::sal_Int32 _monocolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getRadioCheckTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setRadioCheckTextColor( ::sal_Int32 _radiochecktextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getSeparatorColor() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getShadowColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setShadowColor( ::sal_Int32 _shadowcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getWindowColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setWindowColor( ::sal_Int32 _windowcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getWindowTextColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setWindowTextColor( ::sal_Int32 _windowtextcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getWorkspaceColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setWorkspaceColor( ::sal_Int32 _workspacecolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getHighContrastMode() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHighContrastMode( ::sal_Bool _highcontrastmode ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getApplicationFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setApplicationFont( const ::com::sun::star::awt::FontDescriptor& _applicationfont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getHelpFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHelpFont( const ::com::sun::star::awt::FontDescriptor& _helpfont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getTitleFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTitleFont( const ::com::sun::star::awt::FontDescriptor& _titlefont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getFloatTitleFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFloatTitleFont( const ::com::sun::star::awt::FontDescriptor& _floattitlefont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getMenuFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMenuFont( const ::com::sun::star::awt::FontDescriptor& _menufont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getToolFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setToolFont( const ::com::sun::star::awt::FontDescriptor& _toolfont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getGroupFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setGroupFont( const ::com::sun::star::awt::FontDescriptor& _groupfont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getLabelFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLabelFont( const ::com::sun::star::awt::FontDescriptor& _labelfont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getInfoFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setInfoFont( const ::com::sun::star::awt::FontDescriptor& _infofont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getRadioCheckFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setRadioCheckFont( const ::com::sun::star::awt::FontDescriptor& _radiocheckfont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getPushButtonFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPushButtonFont( const ::com::sun::star::awt::FontDescriptor& _pushbuttonfont ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getFieldFont() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFieldFont( const ::com::sun::star::awt::FontDescriptor& _fieldfont ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addStyleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XStyleChangeListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeStyleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XStyleChangeListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    private:
        ::boost::scoped_ptr< WindowStyleSettings_Data > m_pData;
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // TOOLKIT_STYLESETTINGS_HXX
