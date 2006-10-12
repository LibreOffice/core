/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connpooloptions.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:08:02 $
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

#ifndef _OFFAPP_CONNPOOLOPTIONS_HXX_
#define _OFFAPP_CONNPOOLOPTIONS_HXX_

#include <svtools/solar.hrc>
#define SID_SB_POOLING_ENABLED          (RID_OFA_START + 247)
#define SID_SB_DRIVER_TIMEOUTS          (RID_OFA_START + 248)
#define SID_SB_DB_REGISTER              (RID_OFA_START + 249)

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

//........................................................................
namespace offapp
{
//........................................................................

    //====================================================================
    //= ConnectionPoolOptionsPage
    //====================================================================
    class DriverListControl;
    class ConnectionPoolOptionsPage : public SfxTabPage
    {
        using TabPage::ActivatePage;

    protected:
        FixedLine               m_aFrame;
        CheckBox                m_aEnablePooling;
        FixedText               m_aDriversLabel;
        DriverListControl*      m_pDriverList;
        FixedText               m_aDriverLabel;
        FixedText               m_aDriver;
        CheckBox                m_aDriverPoolingEnabled;
        FixedText               m_aTimeoutLabel;
        NumericField            m_aTimeout;

    protected:
        ConnectionPoolOptionsPage(Window* _pParent, const SfxItemSet& _rAttrSet);

    public:
        static SfxTabPage*  Create(Window* _pParent, const SfxItemSet& _rAttrSet);

        ~ConnectionPoolOptionsPage();

    protected:
        virtual long        Notify( NotifyEvent& _rNEvt );

        virtual BOOL        FillItemSet(SfxItemSet& _rSet);
        virtual void        Reset(const SfxItemSet& _rSet);
        virtual void        ActivatePage( const SfxItemSet& _rSet);

    protected:
        DECL_LINK( OnEnabledDisabled, const CheckBox* );
        DECL_LINK( OnDriverRowChanged, const void* );

        void implInitControls(const SfxItemSet& _rSet, sal_Bool _bFromReset);

        void commitTimeoutField();
    };

//........................................................................
}   // namespace offapp
//........................................................................

#endif // _OFFAPP_CONNPOOLOPTIONS_HXX_


