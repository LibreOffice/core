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

#include <vcl/weld.hxx>
#include "numfmtlb.hxx"

class SvNumberFormatter;
class SwDoc;
namespace sw::mark
{
class IDateFieldmark;
}

/// Dialog to specify the properties of date form field
namespace sw
{
class DateFormFieldDialog : public weld::GenericDialogController
{
private:
    sw::mark::IDateFieldmark* m_pDateField;
    SvNumberFormatter* m_pNumberFormatter;

    std::unique_ptr<SwNumFormatTreeView> m_xFormatLB;

    void Apply();
    void InitControls();

public:
    DateFormFieldDialog(weld::Widget* pParent, sw::mark::IDateFieldmark* pDateField, SwDoc& rDoc);
    virtual ~DateFormFieldDialog() override;

    virtual short run() override
    {
        short nRet = GenericDialogController::run();
        if (nRet == RET_OK)
            Apply();
        return nRet;
    }
};

} // namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
