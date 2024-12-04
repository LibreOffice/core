/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWidget.hxx"
#include "QtExpander.hxx"

class QtInstanceExpander : public QtInstanceWidget, public virtual weld::Expander
{
    Q_OBJECT

    QtExpander* m_pExpander;

public:
    QtInstanceExpander(QtExpander* pExpander);

    virtual void set_label(const OUString& rText);
    virtual OUString get_label() const;
    virtual bool get_expanded() const;
    virtual void set_expanded(bool bExpand);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
