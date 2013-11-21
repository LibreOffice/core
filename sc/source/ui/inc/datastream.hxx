/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/lnkbase.hxx>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace datastreams {
    class CallerThread;
    class ReaderThread;
}
class ScDocShell;
class ScDocument;
class ScRange;
class Window;

typedef std::vector<OString> LinesList;

class DataStream : boost::noncopyable, public sfx2::SvBaseLink
{
    OString ConsumeLine();
    void Decode(const OUString& rURL, const OUString& rRange, const OUString& rMove);
    void MoveData();

public:
    enum MoveEnum { NO_MOVE, RANGE_DOWN, MOVE_DOWN, MOVE_UP };

    static void Set(ScDocShell *pShell, const OUString& rURL,
            const OUString& rRange, sal_Int32 nLimit, const OUString& rMove);

    DataStream(ScDocShell *pShell, const OUString& rURL,
        const OUString& rRange, sal_Int32 nLimit, const OUString& rMove);
    virtual ~DataStream() SAL_OVERRIDE;
    virtual void Edit(Window* , const Link& ) SAL_OVERRIDE;

    bool ImportData();
    void Start();
    void Stop();

private:
    ScDocShell *mpScDocShell;
    ScDocument *mpScDocument;
    MoveEnum meMove;
    bool mbRunning;
    bool mbIsUndoEnabled;
    bool mbValuesInLine;
    LinesList *mpLines;
    size_t mnLinesCount;
    boost::scoped_ptr<ScRange> mpRange;
    boost::scoped_ptr<ScRange> mpStartRange;
    boost::scoped_ptr<ScRange> mpEndRange;
    rtl::Reference<datastreams::CallerThread> mxThread;
    rtl::Reference<datastreams::ReaderThread> mxReaderThread;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
