/*************************************************************************
 *
 *  $RCSfile: paramdialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-27 08:07:40 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _DBAUI_PARAMDIALOG_HXX_
#define _DBAUI_PARAMDIALOG_HXX_

#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif

namespace connectivity
{
    class OSQLParseNode;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

    //==================================================================
    //= OParameterDialog
    //==================================================================
    class OParameterDialog : public ModalDialog
    {
    protected:
        // the controls
        GroupBox        m_aNamesFrame;
        ListBox         m_aAllParams;
        GroupBox        m_aValueFrame;
        Edit            m_aParam;
        PushButton      m_aTravelNext;
        FixedLine       m_aDelimiter;
        OKButton        m_aOKBtn;
        CancelButton    m_aCancelBtn;

        sal_uInt16          m_nCurrentlySelected;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                        m_xParams;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                        m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >
                        m_xFormatter;
        ::connectivity::OSQLParser
                        m_aParser;

        void*       m_pVisitedParams;
            // this is a vector of BOOLs, but as this file is exported we don't want to include the stl here ...
            // TODO: now that we migrated the dialog and do not export the file anymore, give it a better implementation ...
        Timer       m_aResetVisitFlag;
            // we reset the "visited flag" 1 second after and entry has been selected

        sal_Bool    m_bNeedErrorOnCurrent;

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                    m_aFinalValues;     /// the final values as entered by the user

    public:
        OParameterDialog(Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & _rParamContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > & _rConnNFormats,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        ~OParameterDialog();

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                    getValues() const { return m_aFinalValues; }


    protected:
        void Construct();

        ::connectivity::OSQLParseNode* implPredicateTree(::rtl::OUString& _rErrorMessage, const UniString& _rStatement, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxField);

    private:
        DECL_LINK(OnVisitedTimeout, Timer*);
        DECL_LINK(OnValueModified, Control*);
        DECL_LINK(OnEntrySelected, ListBox*);
        DECL_LINK(OnButtonClicked, PushButton*);
        DECL_LINK(OnValueLoseFocus, Control*);
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_PARAMDIALOG_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/26 18:08:45  fs
 *  initial checkin - parameter dialog
 *
 *
 *  Revision 1.0 26.10.00 11:54:07  fs
 ************************************************************************/

