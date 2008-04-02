/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CustomAnimationCreateDialog.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:43:49 $
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

#ifndef _SD_CUSTOMANIMATIONCREATEDIALOG_HXX
#define _SD_CUSTOMANIMATIONCREATEDIALOG_HXX

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#include "CustomAnimationPreset.hxx"
#endif

#ifndef _SV_TABDLG_HXX
#include <vcl/tabdlg.hxx>
#endif

enum PathKind { NONE, CURVE, POLYGON, FREEFORM };

class TabControl;
class OKButton;
class CancelButton;
class HelpButton;

namespace sd {

// --------------------------------------------------------------------

class CustomAnimationCreateTabPage;
class CustomAnimationPane;

class CustomAnimationCreateDialog : public TabDialog
{
    friend class CustomAnimationCreateTabPage;
public:
    CustomAnimationCreateDialog( ::Window* pParent, CustomAnimationPane* pPane, const std::vector< ::com::sun::star::uno::Any >& rTargets, bool bHasText, const ::rtl::OUString& rsPresetId, double fDuration );
    ~CustomAnimationCreateDialog();

    PathKind getCreatePathKind() const;
    CustomAnimationPresetPtr getSelectedPreset() const;
    double getSelectedDuration() const;

private:
    CustomAnimationCreateTabPage* getCurrentPage() const;
    void preview( const CustomAnimationPresetPtr& pPreset ) const;
    void setPosition();
    void storePosition();

    DECL_LINK( implActivatePagekHdl, Control* );
    DECL_LINK( implDeactivatePagekHdl, Control* );

private:
    CustomAnimationPane* mpPane;
    const std::vector< ::com::sun::star::uno::Any >& mrTargets;

    double mfDuration;
    bool mbIsPreview;

    TabControl* mpTabControl;
    OKButton* mpOKButton;
    CancelButton* mpCancelButton;
    HelpButton* mpHelpButton;

    CustomAnimationCreateTabPage* mpTabPages[4];
};

}

#endif // _SD_CUSTOMANIMATIONCREATEDIALOG_HXX
