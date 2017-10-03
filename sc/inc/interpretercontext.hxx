/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_INTERPRETERCONTEXT_HXX
#define INCLUDED_SC_INC_INTERPRETERCONTEXT_HXX

class ScDocument;
class SvNumberFormatter;

struct ScInterpreterContext
{
    const ScDocument& mrDoc;
    SvNumberFormatter* mpFormatter;

    ScInterpreterContext(const ScDocument& rDoc, SvNumberFormatter* pFormatter) :
        mrDoc(rDoc),
        mpFormatter(pFormatter)
    {
    }

    ~ScInterpreterContext()
    {
    }

    SvNumberFormatter* GetFormatTable() const
    {
        return mpFormatter;
    }
};

#endif // INCLUDED_SC_INC_INTERPRETERCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
