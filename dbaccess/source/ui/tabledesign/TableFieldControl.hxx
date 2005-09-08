/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableFieldControl.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:41:43 $
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
#ifndef DBAUI_TABLEFIELDCONTROL_HXX
#define DBAUI_TABLEFIELDCONTROL_HXX

#ifndef DBAUI_FIELDDESCRIPTIONCONTROL_HXX
#include "FieldDescControl.hxx"
#endif

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
        virtual void        CellModified(long nRow, USHORT nColId );
        virtual BOOL        IsReadOnly();
        virtual void        SetModified(BOOL bModified);
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


