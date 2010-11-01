/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef DBAUI_DLGSAVE_HXX
#define DBAUI_DLGSAVE_HXX

#include "apitools.hxx"
#include <vcl/dialog.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/msgbox.hxx>

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameAccess;
        class XHierarchicalNameAccess;
    }
    namespace sdbc {
        class XDatabaseMetaData;
        class XConnection;
    }
}}}


#define SAD_DEFAULT                 0x0000
#define SAD_ADDITIONAL_DESCRIPTION  0x0001

#define SAD_TITLE_STORE_AS          0x0000
#define SAD_TITLE_PASTE_AS          0x0100
#define SAD_TITLE_RENAME            0x0200

class Button;
class Edit;
namespace dbaui
{
    class OSaveAsDlgImpl;
    class IObjectNameCheck;
    class OSaveAsDlg : public ModalDialog
    {
    private:
        OSaveAsDlgImpl* m_pImpl;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;
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
        virtual ~OSaveAsDlg();

        String getName() const;
        String getCatalog() const;
        String getSchema() const;
    private:
        DECL_LINK(ButtonClickHdl, Button *);
        DECL_LINK(EditModifyHdl,  Edit * );

        void implInitOnlyTitle(const String& _rLabel);
        void implInit();
    };
}

#endif // DBAUI_DLGSAVE_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
