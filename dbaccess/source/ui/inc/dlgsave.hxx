/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgsave.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:32:07 $
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
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBAUI_SQLNAMEEDIT_HXX
#include "SqlNameEdit.hxx"
#endif

#define SAD_DEFAULT                 0x0000
#define SAD_ADDITIONAL_DESCRIPTION  0x0001

#define SAD_TITLE_STORE_AS          0x0000
#define SAD_TITLE_PASTE_AS          0x0100
#define SAD_TITLE_RENAME            0x0200

namespace dbaui
{
    class IObjectNameCheck;
    class OSaveAsDlg : public ModalDialog
    {
    private:
        FixedText           m_aDescription;
        FixedText           m_aCatalogLbl;
        OSQLNameComboBox    m_aCatalog;
        FixedText           m_aSchemaLbl;
        OSQLNameComboBox    m_aSchema;
        FixedText           m_aLabel;
        OSQLNameEdit        m_aTitle;
        OKButton            m_aPB_OK;
        CancelButton        m_aPB_CANCEL;
        HelpButton          m_aPB_HELP;
        String              m_aQryLabel;
        String              m_sTblLabel;
        String              m_aName;
        const IObjectNameCheck&
                            m_rObjectNameCheck;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >       m_xMetaData;
        sal_Int32           m_nType;
        sal_Int32           m_nFlags;


    public:
        OSaveAsDlg( Window * pParent,const sal_Int32& _rType,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                    const String& rDefault,
                    const IObjectNameCheck& _rObjectNameCheck,
                    sal_Int32 _nFlags = SAD_DEFAULT | SAD_TITLE_STORE_AS);

        OSaveAsDlg( Window* _pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                    const String& _rDefault,
                    const String& _sLabel,
                    const IObjectNameCheck& _rObjectNameCheck,
                    sal_Int32 _nFlags = SAD_DEFAULT | SAD_TITLE_STORE_AS);

        String getName() const      { return m_aName; }
        String getCatalog() const   { return m_aCatalog.IsVisible() ? m_aCatalog.GetText() : String(); }
        String getSchema() const    { return m_aSchema.IsVisible() ? m_aSchema.GetText() : String(); }
    private:
        DECL_LINK(ButtonClickHdl, Button *);
        DECL_LINK(EditModifyHdl,  Edit * );

        void implInitOnlyTitle(const String& _rLabel);
        void implInit();
    };
}

#endif // DBAUI_DLGSAVE_HXX



