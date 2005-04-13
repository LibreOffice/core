/*************************************************************************
 *
 *  $RCSfile: OfficeControlAccess.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 08:48:45 $
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

#ifndef SVTOOLS_CONTROLACCESS_HXX
#define SVTOOLS_CONTROLACCESS_HXX

#ifndef _SVT_FILEVIEW_HXX
#include <svtools/fileview.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef SVTOOLS_PICKER_CALLBACKS_HXX
#include "pickercallbacks.hxx"
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    // --------------------------------------------------------------------
    namespace InternalFilePickerElementIds
    {
        static const sal_Int16 PUSHBUTTON_HELP = (sal_Int16)0x1000;
        static const sal_Int16 TOOLBOXBUTOON_DEFAULT_LOCATION = (sal_Int16)0x1001;
        static const sal_Int16 TOOLBOXBUTOON_LEVEL_UP = (sal_Int16)0x1002;
        static const sal_Int16 TOOLBOXBUTOON_NEW_FOLDER = (sal_Int16)0x1003;
        static const sal_Int16 FIXEDTEXT_CURRENTFOLDER = (sal_Int16)0x1004;
    }

    // --------------------------------------------------------------------
    /** implements the XControlAccess, XControlInformation and XFilePickerControlAccess for the file picker
    */
    class OControlAccess
    {
        IFilePickerController*  m_pFilePickerController;
        SvtFileView*            m_pFileView;

    public:
        OControlAccess( IFilePickerController* _pController, SvtFileView* _pFileView );

        // XControlAccess implementation
        void setControlProperty( const ::rtl::OUString& _rControlName, const ::rtl::OUString& _rControlProperty, const ::com::sun::star::uno::Any& _rValue );
        ::com::sun::star::uno::Any  getControlProperty( const ::rtl::OUString& _rControlName, const ::rtl::OUString& _rControlProperty );

        // XControlInformation implementation
        ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedControls(  );
        ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedControlProperties( const ::rtl::OUString& _rControlName );
        sal_Bool                                            isControlSupported( const ::rtl::OUString& _rControlName );
        sal_Bool                                            isControlPropertySupported( const ::rtl::OUString& _rControlName, const ::rtl::OUString& _rControlProperty );

        // XFilePickerControlAccess
        void                        setValue( sal_Int16 _nId, sal_Int16 _nCtrlAction, const ::com::sun::star::uno::Any& _rValue );
        ::com::sun::star::uno::Any  getValue( sal_Int16 _nId, sal_Int16 _nCtrlAction ) const;
        void                        setLabel( sal_Int16 _nId, const ::rtl::OUString& _rValue );
        ::rtl::OUString             getLabel( sal_Int16 _nId ) const;
        void                        enableControl( sal_Int16 _nId, sal_Bool _bEnable );

        static void             setHelpURL( Window* _pControl, const ::rtl::OUString& _rURL, sal_Bool _bFileView );
        static ::rtl::OUString  getHelpURL( Window* _pControl, sal_Bool _bFileView );

    private:
        /** implements the various methods for setting properties on controls

            @param _nControlId
                the id of the control
            @param _pControl
                the affected control. Must be the same as referred by <arg>_nControlId</arg>, or NULL.
            @param _nProperty
                the property to set
                See PROPERTY_FLAG_*
            @param _rValue
                the value to set
            @param _bIgnoreIllegalArgument
                if <FALSE/>, an exception will be thrown if the given value is of improper type
        */
        void                        implSetControlProperty(
                                        sal_Int16 _nControlId,
                                        Control* _pControl, sal_Int16 _nProperty, const ::com::sun::star::uno::Any& _rValue,
                                        sal_Bool _bIgnoreIllegalArgument = sal_True );

        Control* implGetControl( const ::rtl::OUString& _rControlName, sal_Int16* _pId = NULL, sal_Int32* _pPropertyMask = NULL ) const SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );

        /** implements the various methods for retrieving properties from controls

            @param _pControl
                the affected control
                @PRECOND not <NULL/>
            @param _nProperty
                the property to retrieve
                See PROPERTY_FLAG_*
            @return
        */
        ::com::sun::star::uno::Any  implGetControlProperty( Control* _pControl, sal_Int16 _nProperty ) const;

        void implDoListboxAction( ListBox* _pListbox, sal_Int16 _nCtrlAction, const ::com::sun::star::uno::Any& _rValue );

    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_CONTROLACCESS_HXX

