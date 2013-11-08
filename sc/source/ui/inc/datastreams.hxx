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

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace datastreams { class CallerThread; }
class ScDocShell;
class ScDocument;
class ScRange;
class SvStream;
class Window;

class DataStreams : boost::noncopyable
{
    ScDocShell *mpScDocShell;
    ScDocument *mpScDocument;
    bool mbMove;
    bool mbRunning;
    bool mbIsUndoEnabled;
    boost::scoped_ptr<ScRange> mpRange;
    boost::scoped_ptr<ScRange> mpEndRange;
    boost::scoped_ptr<SvStream> mpStream;
    rtl::Reference<datastreams::CallerThread> mxThread;

public:
    DataStreams(ScDocShell *pScDocShell);
    virtual ~DataStreams();
    bool ImportData();
    void Move();
    void ShowDialog(Window *pParent);
    void Start();
    void Stop();
    void Set(const OUString& rUrl, bool bIsScript, const OUString& rRange);
    void SetMove(sal_Int32 nLimit);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
