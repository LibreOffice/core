/*************************************************************************
 *
 *  $RCSfile: dlgsave.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:39:23 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_DLGSAVE_HXX
#define DBAUI_DLGSAVE_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

namespace dbaui
{
    class OSaveAsDlg : public ModalDialog
    {
    private:
        FixedText       m_aCatalogLbl;
        Edit            m_aCatalog;
        FixedText       m_aSchemaLbl;
        Edit            m_aSchema;
        FixedText       m_aLabel;
        Edit            m_aTitle;
        OKButton        m_aPB_OK;
        CancelButton    m_aPB_CANCEL;
        HelpButton      m_aPB_HELP;
        String          m_aQryLabel;
        String          m_sTblLabel;
        String          m_aName;
        String          m_aExists;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xNames;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>    m_xMetaData;
        sal_Int32       m_nType;


    public:
        OSaveAsDlg( Window * pParent,const sal_Int32& _rType,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>&  _rxNames,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMetaData,
                    const String& rDefault);

        String getName() const      { return m_aName; }
        String getCatalog() const   { return m_aCatalog.IsVisible() ? m_aCatalog.GetText() : String(); }
        String getSchema() const    { return m_aSchema.IsVisible() ? m_aSchema.GetText() : String(); }
    private:
        DECL_LINK(ButtonClickHdl, Button *);
        DECL_LINK(EditModifyHdl,  Edit * );
    };
}

#endif // DBAUI_DLGSAVE_HXX



