/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: templdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:31:12 $
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
#ifndef _SFX_TEMPLDLG_HXX
#define _SFX_TEMPLDLG_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#include <rsc/rscsfx.hxx>

#include <sfx2/dockwin.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>

class SfxTemplateDialog_Impl;
class SfxTemplateCatalog_Impl;

// class ISfxTemplateCommon ----------------------------------------------

class ISfxTemplateCommon
{
public:
    virtual SfxStyleFamily GetActualFamily() const = 0;
    virtual String GetSelectedEntry() const = 0;
};

// class SfxTemplateDialog -----------------------------------------------

class SfxTemplateDialog : public SfxDockingWindow
{
private:
friend class SfxTemplateDialogWrapper;
friend class SfxTemplateDialog_Impl;

    SfxTemplateDialog_Impl*     pImpl;

    virtual void                DataChanged( const DataChangedEvent& _rDCEvt );
    virtual void                Resize();
    virtual SfxChildAlignment   CheckAlignment( SfxChildAlignment, SfxChildAlignment );
    virtual void                StateChanged( StateChangedType nStateChange );

public:
    SfxTemplateDialog( SfxBindings*, SfxChildWindow*, Window* );
    ~SfxTemplateDialog();

    virtual void                Update();

    ISfxTemplateCommon*         GetISfxTemplateCommon();
    void                        SetParagraphFamily();
};

// class SfxTemplateCatalog ----------------------------------------------

class SfxTemplateCatalog : public SfxModalDialog
{
private:
    SfxTemplateCatalog_Impl *pImpl;

public:
    SfxTemplateCatalog(Window * pParent, SfxBindings *pBindings);
    ~SfxTemplateCatalog();
    friend class SfxTemplateCatalog_Impl;
};

// class SfxTemplateDialogWrapper ----------------------------------------

class SFX2_DLLPUBLIC SfxTemplateDialogWrapper : public SfxChildWindow
{
public:
                SfxTemplateDialogWrapper
                    (Window*,USHORT,SfxBindings*,SfxChildWinInfo*);
                SFX_DECL_CHILDWINDOW(SfxTemplateDialogWrapper);

    void            SetParagraphFamily();
};

#endif

