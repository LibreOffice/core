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

#ifndef _SPEAKERDLG_HXX
#define _SPEAKERDLG_HXX

#ifndef _GOODIES_RECOGNIZER_HXX_
#include <goodies/recognizer.hxx>
#endif

#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

// ---------------
// - SfxSpeakerDlg -
// ---------------

class SfxSpeakerDlg: public ModalDialog
{
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    ListBox         aListBox;
    CharSet         aCharSet;

    OSpeakerManagerRef m_aSpeakerMgr;

public:

    SfxSpeakerDlg(Window *pParent, OSpeakerManagerRef& aSpeakerMgr);
    ~SfxSpeakerDlg();

    virtual short Execute();
};


#endif // _SPEAKER_HXX
