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
#include <address.hxx>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

#include <rangelst.hxx>
#include <documentstreamaccess.hxx>

class ScDocShell;
class ScDocument;
class Window;

namespace sc {

namespace datastreams {
    class CallerThread;
    class ReaderThread;
}

typedef std::vector<OString> LinesList;

class DataStream : boost::noncopyable, public sfx2::SvBaseLink
{
    OString ConsumeLine();
    void MoveData();
    void Text2Doc();
    DECL_LINK( RefreshHdl, void* );

public:
    enum MoveEnum { NO_MOVE, RANGE_DOWN, MOVE_DOWN, MOVE_UP };
    enum { SCRIPT_STREAM = 1, VALUES_IN_LINE = 2 };

    static void MakeToolbarVisible();
    static DataStream* Set(ScDocShell *pShell, const OUString& rURL, const OUString& rRange,
            sal_Int32 nLimit, const OUString& rMove, sal_uInt32 nSettings);

    DataStream(
        ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, const OUString& rMove, sal_uInt32 nSettings);

    virtual ~DataStream();
    // sfx2::SvBaseLink
    virtual sfx2::SvBaseLink::UpdateResult DataChanged(
            const OUString& , const css::uno::Any& ) SAL_OVERRIDE;
    virtual void Edit(Window* , const Link& ) SAL_OVERRIDE;

    ScRange GetRange() const;
    const OUString& GetURL() const { return msURL; }
    const sal_Int32& GetLimit() const { return mnLimit; }
    const OUString& GetMove() const { return msMove; }
    const sal_uInt32& GetSettings() const { return mnSettings; }

    void Decode(
        const OUString& rURL, const ScRange& rRange, sal_Int32 nLimit,
        const OUString& rMove, const sal_uInt32 nSettings);

    bool ImportData();
    void StartImport();
    void StopImport();

private:
    void Repaint();
    void Broadcast();

private:
    ScDocShell *mpDocShell;
    ScDocument *mpDoc;
    DocumentStreamAccess maDocAccess;
    OUString msURL;
    OUString msMove;
    sal_Int32 mnLimit;
    sal_uInt32 mnSettings;
    MoveEnum meMove;
    bool mbRunning;
    bool mbValuesInLine;
    LinesList *mpLines;
    size_t mnLinesCount;
    size_t mnRepaintCounter;
    SCROW mnCurRow;
    ScRange maStartRange;
    ScRangeList maBroadcastRanges;
    boost::scoped_ptr<ScRange> mpEndRange;
    rtl::Reference<datastreams::CallerThread> mxThread;
    rtl::Reference<datastreams::ReaderThread> mxReaderThread;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
