/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: uieventslogger.cxx,v $
*
* $Revision: 1.3 $
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
***********************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include <comphelper/uieventslogger.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/logging/XLogHandler.hpp>
#include <com/sun/star/logging/XLoggerPool.hpp>
#include <com/sun/star/oooimprovement/XCoreController.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/time.h>
#include <rtl/ustrbuf.hxx>


using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::logging;
using namespace com::sun::star::oooimprovement;
using namespace com::sun::star::util;
using namespace rtl;
using namespace cppu;
using namespace osl;

namespace comphelper
{
    // declaration of implementation
    class UiEventsLogger_Impl;
    class UiEventsLogger_Impl : public UiEventsLogger
    {
        private:
            //typedefs and friends
            friend class UiEventsLogger;
            typedef ::boost::shared_ptr<UiEventsLogger_Impl> ptr;

            // instance methods and data
            UiEventsLogger_Impl();
            void initializeLogger();
            void logDispatch(const ::com::sun::star::util::URL& url,
                const Sequence<PropertyValue>& args);
            void logRotated();
            void logVcl(const ::rtl::OUString& parent_id,
                sal_Int32 window_type,
                const ::rtl::OUString& id,
                const ::rtl::OUString& method,
                const ::rtl::OUString& param);
            void rotate();
            void hotRotate();
            void prepareLogHandler();
            void checkIdleTimeout();
            OUString getCurrentPath();
            OUString getRotatedPath();

            bool m_Active;
            TimeValue m_LastLogEventTime;
            Mutex m_LogMutex;
            const OUString m_LogPath;
            const TimeValue m_IdleTimeout;
            sal_Int32 m_SessionLogEventCount;
            Reference<XLogger> m_Logger;
            Reference<XLogHandler> m_LogHandler;
            Reference<XCsvLogFormatter> m_Formatter;

            // static methods and data
            static ptr getInstance();
            static bool shouldActivate();
            static bool getEnabledFromCoreController();
            static bool getEnabledFromCfg();
            static TimeValue getIdleTimeoutFromCfg();
            static OUString getLogPathFromCfg();
            static sal_Int32 findDispatchOriginIdx(const Sequence<PropertyValue>& args);

            static ptr instance;
            static Mutex * singleton_mutex;
            static const sal_Int32 COLUMNS;
            static const OUString CFG_ENABLED;
            static const OUString CFG_IDLETIMEOUT;
            static const OUString CFG_LOGGING;
            static const OUString CFG_LOGPATH;
            static const OUString CFG_OOOIMPROVEMENT;
            static const OUString ETYPE_DISPATCH;
            static const OUString ETYPE_ROTATED;
            static const OUString ETYPE_VCL;
            static const OUString CSSL_CSVFORMATTER;
            static const OUString CSSL_FILEHANDLER;
            static const OUString CSSL_LOGGERPOOL;
            static const OUString CSSO_CORECONTROLLER;
            static const OUString CSST_JOBEXECUTOR;
            static const OUString CSSU_PATHSUB;
            static const OUString LOGGERNAME;
            static const OUString LOGORIGINNAME;
            static const OUString UNKNOWN_ORIGIN;
            static const OUString FN_CURRENTLOG;
            static const OUString FN_ROTATEDLOG;
            static const OUString LOGROTATE_EVENTNAME;
            static const OUString URL_UNO;
    };
}

namespace comphelper
{
    // consts
    const sal_Int32 UiEventsLogger_Impl::COLUMNS = 8;
    const OUString UiEventsLogger_Impl::CFG_ENABLED = OUString::createFromAscii("EnablingAllowed");
    const OUString UiEventsLogger_Impl::CFG_IDLETIMEOUT = OUString::createFromAscii("IdleTimeout");
    const OUString UiEventsLogger_Impl::CFG_LOGGING = OUString::createFromAscii("/org.openoffice.Office.Logging");
    const OUString UiEventsLogger_Impl::CFG_LOGPATH = OUString::createFromAscii("LogPath");
    const OUString UiEventsLogger_Impl::CFG_OOOIMPROVEMENT = OUString::createFromAscii("OOoImprovement");

    const OUString UiEventsLogger_Impl::CSSL_CSVFORMATTER = OUString::createFromAscii("com.sun.star.logging.CsvFormatter");
    const OUString UiEventsLogger_Impl::CSSL_FILEHANDLER = OUString::createFromAscii("com.sun.star.logging.FileHandler");
    const OUString UiEventsLogger_Impl::CSSL_LOGGERPOOL = OUString::createFromAscii("com.sun.star.logging.LoggerPool");
    const OUString UiEventsLogger_Impl::CSSO_CORECONTROLLER = OUString::createFromAscii("com.sun.star.oooimprovement.CoreController");
    const OUString UiEventsLogger_Impl::CSSU_PATHSUB = OUString::createFromAscii("com.sun.star.util.PathSubstitution");

    const OUString UiEventsLogger_Impl::ETYPE_DISPATCH = OUString::createFromAscii("dispatch");
    const OUString UiEventsLogger_Impl::ETYPE_ROTATED = OUString::createFromAscii("rotated");
    const OUString UiEventsLogger_Impl::ETYPE_VCL = OUString::createFromAscii("vcl");

    const OUString UiEventsLogger_Impl::LOGGERNAME = OUString::createFromAscii("org.openoffice.oooimprovement.Core.UiEventsLogger");
    const OUString UiEventsLogger_Impl::LOGORIGINNAME = OUString::createFromAscii("comphelper.UiEventsLogger.LogOrigin");

    const OUString UiEventsLogger_Impl::UNKNOWN_ORIGIN = OUString::createFromAscii("unknown origin");
    const OUString UiEventsLogger_Impl::FN_CURRENTLOG = OUString::createFromAscii("Current");
    const OUString UiEventsLogger_Impl::FN_ROTATEDLOG = OUString::createFromAscii("OOoImprove");
    const OUString UiEventsLogger_Impl::LOGROTATE_EVENTNAME = OUString::createFromAscii("onOOoImprovementLogRotated");

    const OUString UiEventsLogger_Impl::URL_UNO = OUString::createFromAscii(".uno:");


    // public UiEventsLogger interface
    sal_Bool UiEventsLogger::isEnabled()
    {
        try {
            UiEventsLogger_Impl::ptr inst = UiEventsLogger_Impl::getInstance();
            if(inst==UiEventsLogger_Impl::ptr()) return false;
            return inst->m_Active;
        } catch(...) { return false; } // never throws
    }

    sal_Int32 UiEventsLogger::getSessionLogEventCount()
    {
        UiEventsLogger_Impl::ptr inst = UiEventsLogger_Impl::getInstance();
        if(inst==UiEventsLogger_Impl::ptr()) return 0;
        return inst->m_SessionLogEventCount;
    }

    void UiEventsLogger::appendDispatchOrigin(
        Sequence<PropertyValue>& args,
        const OUString& origin)
    {
        sal_Int32 old_length = args.getLength();
        args.realloc(old_length+1);
        args[old_length].Name = UiEventsLogger_Impl::LOGORIGINNAME;
        args[old_length].Value = static_cast<Any>(origin);
    }

    Sequence<PropertyValue> UiEventsLogger::purgeDispatchOrigin(
        const Sequence<PropertyValue>& args)
    {
        if(args.getLength()==0) return args;
        sal_Int32 idx = UiEventsLogger_Impl::findDispatchOriginIdx(args);
        if(idx==-1) return args;
        Sequence<PropertyValue> result(args);
        if(idx!=result.getLength()-1)
            result[idx] = result[result.getLength()-1];
        result.realloc(result.getLength()-1);
        return result;
    }

    void UiEventsLogger::logDispatch(
        const URL& url,
        const Sequence<PropertyValue>& args)
    {
        try {
            UiEventsLogger_Impl::ptr inst = UiEventsLogger_Impl::getInstance();
            if(inst!=UiEventsLogger_Impl::ptr()) inst->logDispatch(url, args);
        } catch(...) { } // never throws
    }

    void UiEventsLogger::logVcl(
        const OUString& parent_id,
        sal_Int32 window_type,
        const OUString& id,
        const OUString& method,
        const OUString& param)
    {
        try {
            UiEventsLogger_Impl::ptr inst = UiEventsLogger_Impl::getInstance();
            if(inst!=UiEventsLogger_Impl::ptr()) inst->logVcl(parent_id, window_type, id, method, param);
        } catch(...) { } // never throws
    }

    void UiEventsLogger::logVcl(
        const OUString& parent_id,
        sal_Int32 window_type,
        const OUString& id,
        const OUString& method,
        sal_Int32 param)
    {
        OUStringBuffer buf;
        UiEventsLogger::logVcl(parent_id, window_type, id, method, buf.append(param).makeStringAndClear());
    }

    void UiEventsLogger::logVcl(
        const OUString& parent_id,
        sal_Int32 window_type,
        const OUString& id,
        const OUString& method)
    {
        OUString empty;
        UiEventsLogger::logVcl(parent_id, window_type, id, method, empty);
    }

    // private UiEventsLogger_Impl methods
    UiEventsLogger_Impl::UiEventsLogger_Impl()
        : m_Active(UiEventsLogger_Impl::shouldActivate())
        , m_LogPath(UiEventsLogger_Impl::getLogPathFromCfg())
        , m_IdleTimeout(UiEventsLogger_Impl::getIdleTimeoutFromCfg())
        , m_SessionLogEventCount(0)
    {
        m_LastLogEventTime.Seconds = m_LastLogEventTime.Nanosec = 0;
        if(m_Active) rotate();
        if(m_Active) initializeLogger();
    }

    void UiEventsLogger_Impl::logDispatch(
        const URL& url,
        const Sequence<PropertyValue>& args)
    {
        if(!m_Active) return;
        if(!url.Complete.match(URL_UNO)) return;
        Guard<Mutex> log_guard(m_LogMutex);
        checkIdleTimeout();

        Sequence<OUString> logdata = Sequence<OUString>(COLUMNS);
        logdata[0] = ETYPE_DISPATCH;
        sal_Int32 origin_idx = findDispatchOriginIdx(args);
        if(origin_idx!=-1)
            args[origin_idx].Value >>= logdata[1];
        else
            logdata[1] = UNKNOWN_ORIGIN;
        logdata[2] = url.Complete;
        m_Logger->log(LogLevel::INFO, m_Formatter->formatMultiColumn(logdata));
        m_SessionLogEventCount++;
    }

    void UiEventsLogger_Impl::logRotated()
    {
        Sequence<OUString> logdata = Sequence<OUString>(COLUMNS);
        logdata[0] = ETYPE_ROTATED;
        m_Logger->log(LogLevel::INFO, m_Formatter->formatMultiColumn(logdata));
    }

    void UiEventsLogger_Impl::logVcl(
        const OUString& parent_id,
        sal_Int32 window_type,
        const OUString& id,
        const OUString& method,
        const OUString& param)
    {
        if(!m_Active) return;
        Guard<Mutex> log_guard(m_LogMutex);
        checkIdleTimeout();

        OUStringBuffer buf;
        Sequence<OUString> logdata = Sequence<OUString>(COLUMNS);
        logdata[0] = ETYPE_VCL;
        logdata[3] = parent_id;
        logdata[4] = buf.append(window_type).makeStringAndClear();
        logdata[5] = id;
        logdata[6] = method;
        logdata[7] = param;
        m_Logger->log(LogLevel::INFO, m_Formatter->formatMultiColumn(logdata));
        m_SessionLogEventCount++;
    }

    void UiEventsLogger_Impl::rotate()
    {
        FileBase::RC result = File::move(getCurrentPath(), getRotatedPath());
        if(result!=FileBase::E_None && result!=FileBase::E_NOENT)
            m_Active = false;
    }

    void UiEventsLogger_Impl::hotRotate()
    {
        logRotated();
        m_Logger->removeLogHandler(m_LogHandler);
        m_LogHandler = NULL;
        rotate();
        prepareLogHandler();
        if(m_Formatter.is() && m_LogHandler.is() && m_Logger.is())
        {
            m_LogHandler->setFormatter(Reference<XLogFormatter>(m_Formatter, UNO_QUERY));
            m_LogHandler->setLevel(LogLevel::ALL);
            m_Logger->addLogHandler(m_LogHandler);
        }
        else
            m_Active = false;
    }

    void UiEventsLogger_Impl::prepareLogHandler()
    {
        Reference<XMultiServiceFactory> sm = getProcessServiceFactory();

        Sequence<Any> init_args = Sequence<Any>(1);
        init_args[0] = static_cast<Any>(getCurrentPath());
        Reference< XInterface > temp =
            sm->createInstanceWithArguments(CSSL_FILEHANDLER, init_args);
        m_LogHandler = Reference<XLogHandler>(temp, UNO_QUERY);
    }

    void UiEventsLogger_Impl::checkIdleTimeout()
    {
        TimeValue now;
        osl_getSystemTime(&now);
        if(now.Seconds - m_LastLogEventTime.Seconds > m_IdleTimeout.Seconds && m_SessionLogEventCount>0)
            hotRotate();
        m_LastLogEventTime = now;
    }

    OUString UiEventsLogger_Impl::getCurrentPath()
    {
        OUStringBuffer current_path(m_LogPath);
        current_path.appendAscii("/");
        current_path.append(FN_CURRENTLOG);
        current_path.appendAscii(".csv");
        return current_path.makeStringAndClear();
    }

    OUString UiEventsLogger_Impl::getRotatedPath()
    {
        OUStringBuffer rotated_path(m_LogPath);
        rotated_path.appendAscii("/");
        rotated_path.append(FN_ROTATEDLOG);
        rotated_path.appendAscii("-");
        {
            // ISO 8601
            char tsrotated_pathfer[20];
            oslDateTime now;
            TimeValue now_tv;
            osl_getSystemTime(&now_tv);
            osl_getDateTimeFromTimeValue(&now_tv, &now);
            const size_t rotated_pathfer_size = sizeof(tsrotated_pathfer);
            snprintf(tsrotated_pathfer, rotated_pathfer_size, "%04i-%02i-%02iT%02i_%02i_%02i",
                now.Year,
                now.Month,
                now.Day,
                now.Hours,
                now.Minutes,
                now.Seconds);
            rotated_path.appendAscii(tsrotated_pathfer);
            rotated_path.appendAscii(".csv");
        }
        return rotated_path.makeStringAndClear();
    }

    void UiEventsLogger_Impl::initializeLogger()
    {
        Reference<XMultiServiceFactory> sm = getProcessServiceFactory();

        // getting the LoggerPool
        Reference<XLoggerPool> pool;
        {
            Reference<XInterface> temp =
                sm->createInstance(CSSL_LOGGERPOOL);
            pool = Reference<XLoggerPool>(temp, UNO_QUERY);
        }

        // getting the Logger
        m_Logger = pool->getNamedLogger(LOGGERNAME);

        // getting the FileHandler
        prepareLogHandler();

        // getting the Formatter
        {
            Reference<XInterface> temp =
                sm->createInstance(CSSL_CSVFORMATTER);
            m_Formatter = Reference<XCsvLogFormatter>(temp, UNO_QUERY);
        }

        if(m_Formatter.is() && m_LogHandler.is() && m_Logger.is())
        {
            Sequence<OUString> columns = Sequence<OUString>(COLUMNS);
            columns[0] = OUString::createFromAscii("eventtype");
            columns[1] = OUString::createFromAscii("origin");
            columns[2] = OUString::createFromAscii("uno url");
            columns[3] = OUString::createFromAscii("parent id");
            columns[4] = OUString::createFromAscii("window type");
            columns[5] = OUString::createFromAscii("id");
            columns[6] = OUString::createFromAscii("method");
            columns[7] = OUString::createFromAscii("parameter");
            m_Formatter->setColumnnames(columns);
            m_LogHandler->setFormatter(Reference<XLogFormatter>(m_Formatter, UNO_QUERY));
            m_Logger->setLevel(LogLevel::ALL);
            m_LogHandler->setLevel(LogLevel::ALL);
            m_Logger->addLogHandler(m_LogHandler);
        }
        else
            m_Active = false;
    }

    // private static UiEventsLogger_Impl
    bool UiEventsLogger_Impl::shouldActivate()
    {
        return getEnabledFromCfg() && getEnabledFromCoreController();
    }

    OUString UiEventsLogger_Impl::getLogPathFromCfg()
    {
        OUString result;
        Reference<XMultiServiceFactory> sm = getProcessServiceFactory();

        ConfigurationHelper::readDirectKey(
            sm,
            CFG_LOGGING, CFG_OOOIMPROVEMENT, CFG_LOGPATH,
            ConfigurationHelper::E_READONLY
        ) >>= result;

        Reference<XStringSubstitution> path_sub(
            sm->createInstance(CSSU_PATHSUB),
            UNO_QUERY);
        if(path_sub.is())
            result = path_sub->substituteVariables(result, sal_False);
        return result;
    }

    TimeValue UiEventsLogger_Impl::getIdleTimeoutFromCfg()
    {
        sal_Int32 timeoutminutes = 360;
        Reference<XMultiServiceFactory> sm = getProcessServiceFactory();

        ConfigurationHelper::readDirectKey(
            sm,
            CFG_LOGGING, CFG_OOOIMPROVEMENT, CFG_IDLETIMEOUT,
            ConfigurationHelper::E_READONLY
        ) >>= timeoutminutes;
        TimeValue result;
        result.Seconds = static_cast<sal_uInt32>(timeoutminutes)*60;
        result.Nanosec = 0;
        return result;
    }

    bool UiEventsLogger_Impl::getEnabledFromCfg()
    {
        sal_Bool result = false;
        Reference<XMultiServiceFactory> sm = getProcessServiceFactory();
        ConfigurationHelper::readDirectKey(
            sm,
            CFG_LOGGING, CFG_OOOIMPROVEMENT, CFG_ENABLED,
            ::comphelper::ConfigurationHelper::E_READONLY
        ) >>= result;
        return result;
    }

    bool UiEventsLogger_Impl::getEnabledFromCoreController()
    {
        Reference<XMultiServiceFactory> sm = getProcessServiceFactory();
        Reference<XCoreController> core_c(
            sm->createInstance(OUString::createFromAscii("com.sun.star.oooimprovement.CoreController")),
            UNO_QUERY);
        if(!core_c.is()) return false;
        return core_c->enablingUiEventsLoggerAllowed(1);
    }

    UiEventsLogger_Impl::ptr UiEventsLogger_Impl::instance = UiEventsLogger_Impl::ptr();
    Mutex * UiEventsLogger_Impl::singleton_mutex = NULL;
    UiEventsLogger_Impl::ptr UiEventsLogger_Impl::getInstance()
    {
        if(singleton_mutex==NULL)
        {
            Guard<Mutex> global_guard(Mutex::getGlobalMutex());
            if(singleton_mutex==NULL)
                singleton_mutex = new Mutex();
        }
        Guard<Mutex> singleton_guard(singleton_mutex);
        if(instance == 0)
            instance = UiEventsLogger_Impl::ptr(new UiEventsLogger_Impl());
        return instance;
    }

    sal_Int32 UiEventsLogger_Impl::findDispatchOriginIdx(const Sequence<PropertyValue>& args)
    {
        for(sal_Int32 i=0; i<args.getLength(); i++)
            if(args[i].Name == LOGORIGINNAME)
                return i;
        return -1;
    }
}
