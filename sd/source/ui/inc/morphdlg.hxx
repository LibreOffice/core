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

#ifndef SD_MORPH_DLG_HXX
#define SD_MORPH_DLG_HXX

#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>

class SdrObject;

namespace sd {

class MorphDlg
    : public ModalDialog
{
public:
    MorphDlg (
        ::Window* pParent,
        const SdrObject* pObj1,
        const SdrObject* pObj2);
    virtual ~MorphDlg (void);

    void            SaveSettings() const;
    sal_uInt16          GetFadeSteps() const { return (sal_uInt16) aMtfSteps.GetValue(); }
    sal_Bool            IsAttributeFade() const { return aCbxAttributes.IsChecked(); }
    sal_Bool            IsOrientationFade() const { return aCbxOrientation.IsChecked(); }

private:
    FixedLine       aGrpPreset;
    FixedText       aFtSteps;
    MetricField     aMtfSteps;
    CheckBox        aCbxAttributes;
    CheckBox        aCbxOrientation;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    void            LoadSettings();
};


#endif


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
