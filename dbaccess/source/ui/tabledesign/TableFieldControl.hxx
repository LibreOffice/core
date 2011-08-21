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
#ifndef DBAUI_TABLEFIELDCONTROL_HXX
#define DBAUI_TABLEFIELDCONTROL_HXX

#include "FieldDescControl.hxx"

namespace dbaui
{
    class OTableEditorCtrl;
    class OTableDesignHelpBar;
    //==================================================================
    // OTableFieldControl
    //==================================================================
    class OTableFieldControl : public OFieldDescControl
    {
        OTableEditorCtrl*   GetCtrl() const;
    protected:
        virtual void        ActivateAggregate( EControlType eType );
        virtual void        DeactivateAggregate( EControlType eType );
        // Sind von den abgeleiteten Klassen zu impl.
        virtual void        CellModified(long nRow, sal_uInt16 nColId );
        virtual sal_Bool        IsReadOnly();
        virtual void        SetModified(sal_Bool bModified);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    GetFormatter() const;

        virtual ::com::sun::star::lang::Locale  GetLocale() const;

        virtual TOTypeInfoSP                    getTypeInfo(sal_Int32 _nPos);
        virtual const OTypeInfoMap*             getTypeInfo() const;
        virtual sal_Bool                        isAutoIncrementValueEnabled() const;
        virtual ::rtl::OUString                 getAutoIncrementValue() const;

    public:
        OTableFieldControl( Window* pParent, OTableDesignHelpBar* pHelpBar);

        String BoolStringPersistent(const String& rUIString) const { return OFieldDescControl::BoolStringPersistent(rUIString); }
        String BoolStringUI(const String& rPersistentString) const { return OFieldDescControl::BoolStringUI(rPersistentString); }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection();
    };
}
#endif // DBAUI_TABLEFIELDCONTROL_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
