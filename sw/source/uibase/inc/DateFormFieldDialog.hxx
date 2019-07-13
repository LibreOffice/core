/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DATEFORMFIELDDIALOG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DATEFORMFIELDDIALOG_HXX

#include <svx/stddlg.hxx>
#include "actctrl.hxx"
#include "numfmtlb.hxx"

class SvNumberFormatter;
class SwDoc;

namespace sw
{
namespace mark
{
class IDateFieldmark;
}
} // namespace sw

/// Dialog to specify the properties of date form field
namespace sw
{
class DateFormFieldDialog : public SvxStandardDialog
{
private:
    sw::mark::IDateFieldmark* m_pDateField;
    SvNumberFormatter* m_pNumberFormatter;

    VclPtr<NumFormatListBox> m_xFormatLB;

    virtual void Apply() override;
    void InitControls();

public:
    DateFormFieldDialog(vcl::Window* pParent, sw::mark::IDateFieldmark* pDateField, SwDoc* pDoc);
    virtual ~DateFormFieldDialog() override;
    virtual void dispose() override;
};

} // namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
