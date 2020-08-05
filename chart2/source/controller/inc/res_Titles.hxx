/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include "TitleDialogData.hxx"

template <typename Arg, typename Ret> class Link;
namespace weld { class Builder; }
namespace weld { class Entry; }
namespace weld { class Label; }

namespace chart
{

class TitleResources final
{
public:
    TitleResources(weld::Builder& rParent, bool bShowSecondaryAxesTitle);
    ~TitleResources();

    void writeToResources( const TitleDialogData& rInput );
    void readFromResources( TitleDialogData& rOutput );

    void connect_changed( const Link<weld::Entry&,void>& rLink );
    bool get_value_changed_from_saved() const;
    void save_value();

private:
    std::unique_ptr<weld::Label> m_xFT_Main;
    std::unique_ptr<weld::Label> m_xFT_Sub;
    std::unique_ptr<weld::Entry> m_xEd_Main;
    std::unique_ptr<weld::Entry> m_xEd_Sub;

    std::unique_ptr<weld::Label> m_xFT_XAxis;
    std::unique_ptr<weld::Label> m_xFT_YAxis;
    std::unique_ptr<weld::Label> m_xFT_ZAxis;
    std::unique_ptr<weld::Entry> m_xEd_XAxis;
    std::unique_ptr<weld::Entry> m_xEd_YAxis;
    std::unique_ptr<weld::Entry> m_xEd_ZAxis;

    std::unique_ptr<weld::Label> m_xFT_SecondaryXAxis;
    std::unique_ptr<weld::Label> m_xFT_SecondaryYAxis;
    std::unique_ptr<weld::Entry> m_xEd_SecondaryXAxis;
    std::unique_ptr<weld::Entry> m_xEd_SecondaryYAxis;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
