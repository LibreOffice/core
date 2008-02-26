/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: new.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:58:11 $
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
#ifndef _SFXNEW_HXX
#define _SFXNEW_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <sfx2/basedlgs.hxx>

//=========================================================================

class SfxObjectShellLock;
class SfxObjectShell;
class MoreButton;

//=========================================================================

#define SFXWB_PREVIEW 0x0003
#define SFXWB_LOAD_TEMPLATE 0x0004

#define SFX_LOAD_TEXT_STYLES    0x0001
#define SFX_LOAD_FRAME_STYLES   0x0002
#define SFX_LOAD_PAGE_STYLES    0x0004
#define SFX_LOAD_NUM_STYLES     0x0008
#define SFX_MERGE_STYLES        0x0010

#define RET_TEMPLATE_LOAD       100

class SFX2_DLLPUBLIC SfxPreviewWin: public Window
{
    SfxObjectShellLock &rDocShell;
protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    /** state whether a system setting for high contrast should be evaluated
        and taken into account for this window.

        The default implementation uses the accessibility option IsForDrawings
      */
    virtual bool    UseHighContrastSetting() const;

public:
    SfxPreviewWin( Window* pParent,
                   const ResId& rResId,
                   SfxObjectShellLock &rDocSh );
};

class SfxNewFileDialog_Impl;
class SFX2_DLLPUBLIC SfxNewFileDialog : public SfxModalDialog
{
    friend class SfxNewFileDialog_Impl;

private:
    SfxNewFileDialog_Impl* pImpl;

public:

    SfxNewFileDialog(Window *pParent, USHORT nFlags = 0);
    ~SfxNewFileDialog();

        // Liefert FALSE, wenn '- Keine -' als Vorlage eingestellt ist
        // Nur wenn IsTemplate() TRUE liefert, koennen Vorlagennamen
        // erfragt werden
    BOOL IsTemplate() const;
    String GetTemplateRegion() const;
    String GetTemplateName() const;
    String GetTemplateFileName() const;

    // load template methods
    USHORT  GetTemplateFlags()const;
    void    SetTemplateFlags(USHORT nSet);
};

#endif
