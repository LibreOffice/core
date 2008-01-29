/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblefactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:04:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef TOOLKIT_HELPER_ACCESSIBLE_FACTORY_HXX
#define TOOLKIT_HELPER_ACCESSIBLE_FACTORY_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _RTL_REF_HXX
#include <rtl/ref.hxx>
#endif

namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
    class XAccessibleContext;
} } } }
class VCLXButton;
class VCLXCheckBox;
class VCLXRadioButton;
class VCLXListBox;
class VCLXFixedHyperlink;
class VCLXFixedText;
class VCLXScrollBar;
class VCLXEdit;
class VCLXComboBox;
class VCLXToolBox;
class VCLXWindow;
class Menu;

//........................................................................
namespace toolkit
{
//........................................................................

    /** a function which is able to create a factory for the standard Accessible/Context
        components needed for standard toolkit controls

        The returned pointer denotes an instance of the IAccessibleFactory, which has been acquired
        <em>once</em>. The caller is responsible for holding this reference as long as it needs the
        factory, and release it afterwards.
    */
    typedef void* (SAL_CALL * GetStandardAccComponentFactory)( );

    //================================================================
    //= IAccessibleFactory
    //================================================================
    class IAccessibleFactory : public ::rtl::IReference
    {
    public:
        /** creates an accessible context for a button window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXButton* _pXWindow ) = 0;

        /** creates an accessible context for a checkbox window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXCheckBox* _pXWindow ) = 0;

        /** creates an accessible context for a radio button window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXRadioButton* _pXWindow ) = 0;

        /** creates an accessible context for a listbox window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXListBox* _pXWindow ) = 0;

        /** creates an accessible context for a fixed hyperlink window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXFixedHyperlink* _pXWindow ) = 0;

        /** creates an accessible context for a fixed text window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXFixedText* _pXWindow ) = 0;

        /** creates an accessible context for a scrollbar window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXScrollBar* _pXWindow ) = 0;

        /** creates an accessible context for a edit window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXEdit* _pXWindow ) = 0;

        /** creates an accessible context for a combo box window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXComboBox* _pXWindow ) = 0;

        /** creates an accessible context for a toolbox window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXToolBox* _pXWindow ) = 0;

        /** creates an accessible context for a generic window
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleContext( VCLXWindow* _pXWindow ) = 0;

        /** creates an accessible component for the given menu
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessible( Menu* _pMenu, sal_Bool _bIsMenuBar ) = 0;
    };

//........................................................................
}   // namespace toolkit
//........................................................................

#endif // TOOLKIT_HELPER_ACCESSIBLE_FACTORY_HXX
