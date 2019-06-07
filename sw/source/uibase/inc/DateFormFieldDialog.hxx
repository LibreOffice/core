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
#include "actctrl.hxx"
#include "numfmtlb.hxx"

namespace sw
{
namespace mark
{
class IFieldmark;
}
} // namespace sw

/// Dialog to specify the properties of date form field
namespace sw
{
class DateFormFieldDialog : public weld::GenericDialogController
{
private:
    mark::IFieldmark* m_pDateField;

    std::unique_ptr<SwNumFormatTreeView> m_xFormatLB;

    void Apply();
    void InitControls();

public:
    DateFormFieldDialog(weld::Widget* pParent, mark::IFieldmark* pDateField);
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
