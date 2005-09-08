/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SqlNameEdit.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:34:26 $
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
#ifndef DBAUI_SQLNAMEEDIT_HXX
#define DBAUI_SQLNAMEEDIT_HXX

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

namespace dbaui
{
    class OSQLNameChecker
    {
        ::rtl::OUString m_sAllowedChars;
        sal_Bool        m_bOnlyUpperCase;
        sal_Bool        m_bCheck;           // true when we should check for invalid chars
    public:
        OSQLNameChecker(const ::rtl::OUString& _rAllowedChars)
            :m_sAllowedChars(_rAllowedChars)
            ,m_bOnlyUpperCase(sal_False)
            ,m_bCheck(sal_True)
        {
        }

        void setUpperCase(sal_Bool _bUpper=sal_True)
        {
            m_bOnlyUpperCase = _bUpper;
        }
        void setAllowedChars(const ::rtl::OUString& _rAllowedChars)
        {
            m_sAllowedChars = _rAllowedChars;
        }
        // default is false because it is initialized with true
        void setCheck(sal_Bool _bCheck = sal_False)
        {
            m_bCheck = _bCheck;
        }
        sal_Bool checkString(const ::rtl::OUString& _sOldValue,const ::rtl::OUString& _sToCheck,::rtl::OUString& _rsCorrected);
    };
    //==================================================================
    class OSQLNameEdit : public Edit
                        ,public OSQLNameChecker
    {
    public:
        OSQLNameEdit(Window* _pParent,const ::rtl::OUString& _rAllowedChars, WinBits nStyle = WB_BORDER)
            : Edit(_pParent,nStyle)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }
        OSQLNameEdit(Window* _pParent,const ResId& _rRes,const ::rtl::OUString& _rAllowedChars = ::rtl::OUString())
            : Edit(_pParent,_rRes)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }

        // Window overload
        //  virtual long PreNotify( NotifyEvent& rNEvt );
        // Edit
        virtual void Modify();
    };

    class OSQLNameComboBox : public ComboBox
                            ,public OSQLNameChecker
    {
    public:
        OSQLNameComboBox(Window* _pParent,const ::rtl::OUString& _rAllowedChars, WinBits nStyle = WB_BORDER)
            : ComboBox(_pParent,nStyle)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }
        OSQLNameComboBox(Window* _pParent,const ResId& _rRes,const ::rtl::OUString& _rAllowedChars = ::rtl::OUString())
            : ComboBox(_pParent,_rRes)
            ,OSQLNameChecker(_rAllowedChars)
        {
        }

        // Window overload
        // Edit
        virtual void Modify();
    };

}
#endif // DBAUI_SQLNAMEEDIT_HXX


