/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: warnbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:05:42 $
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

#ifndef SC_WARNBOX_HXX
#define SC_WARNBOX_HXX

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif


// ============================================================================

/** Message box with warning image and "Do not show again" checkbox. */
class ScCbWarningBox : public WarningBox
{
public:
    /** @param rMsgStr  Resource ID for the message text.
        @param bDefYes  true = "Yes" focused, false = "No" focused. */
                                ScCbWarningBox( Window* pParent, const String& rMsgStr, bool bDefYes = true );

    /** Opens dialog if IsDialogEnabled() returns true.
        @descr  If after executing the dialog the checkbox "Do not show again" is set,
                the method DisableDialog() will be called. */
    virtual sal_Int16           Execute();

    /** Called before executing the dialog. If this method returns false, the dialog will not be opened. */
    virtual bool                IsDialogEnabled();
    /** Called, when dialog is exited and the option "Do not show again" is set. */
    virtual void                DisableDialog();
};


// ----------------------------------------------------------------------------

/** Warning box for "Replace cell contents?". */
class ScReplaceWarnBox : public ScCbWarningBox
{
public:
                                ScReplaceWarnBox( Window* pParent );

    /** Reads the configuration key "ReplaceCellsWarning". */
    virtual bool                IsDialogEnabled();
    /** Sets the configuration key "ReplaceCellsWarning" to false. */
    virtual void                DisableDialog();
};


// ============================================================================

#endif

