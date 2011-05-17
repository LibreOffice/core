/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
 *
***********************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include <comphelper/uieventslogger.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/logging/XCsvLogFormatter.hpp>
#include <com/sun/star/logging/XLogHandler.hpp>
#include <com/sun/star/logging/XLogger.hpp>
#include <com/sun/star/logging/XLoggerPool.hpp>
#include <com/sun/star/oooimprovement/XCoreController.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <map>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/time.h>
#include <rtl/ustrbuf.hxx>


using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::logging;
using namespace com::sun::star::oooimprovement;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;
using namespace osl;
using namespace std;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace
{
    static void lcl_SetupOriginAppAbbr(map<OUString, OUString>& abbrs)
    {
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextDocument"))] = OUString(sal_Unicode('W')); // Writer
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDocument"))] = OUString(sal_Unicode('C')); // Calc
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationDocument"))] = OUString(sal_Unicode('I')); // Impress
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocument"))] = OUString(sal_Unicode('D')); // Draw
    };

    static void lcl_SetupOriginWidgetAbbr(map<OUString,OUString>& abbrs)
    {
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("ButtonToolbarController"))] = OUString(sal_Unicode('0'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("ComplexToolbarController"))] = OUString(sal_Unicode('1'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("ControlMenuController"))] = OUString(sal_Unicode('2'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("FontMenuController"))] = OUString(sal_Unicode('3'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("FontSizeMenuController"))] = OUString(sal_Unicode('4'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("FooterMenuController"))] = OUString(sal_Unicode('5'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("GenericToolbarController"))] = OUString(sal_Unicode('6'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderMenuController"))] = OUString(sal_Unicode('7'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("LanguageSelectionMenuController"))] = OUString(sal_Unicode('8'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("LangSelectionStatusbarController"))] = OUString(sal_Unicode('9'));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("MacrosMenuController"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("10"));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("MenuBarManager"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("11"));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("NewMenuController"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("12"));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("ObjectMenuController"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("13"));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("RecentFilesMenuController"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("14"));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("ToolbarsMenuController"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("15"));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("SfxToolBoxControl"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("16"));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("SfxAsyncExec"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("17"));
        abbrs[OUString(RTL_CONSTASCII_USTRINGPARAM("AcceleratorExecute"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("18"));
    };
}

namespace comphelper
{
    // declaration of implementation
    class UiEventsLogger_Impl;
    class UiEventsLogger_Impl : public UiEventsLogger
    {
        private:
            //typedefs and friends
            friend class UiEventsLogger;
            typedef UiEventsLogger_Impl* ptr;

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
            void disposing();

            bool m_Active;
            TimeValue m_LastLogEventTime;
            const OUString m_LogPath;
            const TimeValue m_IdleTimeout;
            sal_Int32 m_SessionLogEventCount;
            Reference<XLogger> m_Logger;
            Reference<XLogHandler> m_LogHandler;
            Reference<XCsvLogFormatter> m_Formatter;
            map<OUString, OUString> m_OriginAppAbbr;
            map<OUString, OUString> m_OriginWidgetAbbr;


            // static methods and data
            static ptr getInstance();
            static void prepareMutex();
            static bool shouldActivate();
            static bool getEnabledFromCoreController();
            static bool getEnabledFromCfg();
            static TimeValue getIdleTimeoutFromCfg();
            static OUString getLogPathFromCfg();
            static sal_Int32 findIdx(const Sequence<PropertyValue>& args, const OUString& key);

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
            static const OUString LOGORIGINAPP;
            static const OUString LOGORIGINWIDGET;
            static const OUString UNKNOWN_ORIGIN;
            static const OUString FN_CURRENTLOG;
            static const OUString FN_ROTATEDLOG;
            static const OUString LOGROTATE_EVENTNAME;
            static const OUString URL_UNO;
            static const OUString URL_SPECIAL;
            static const OUString URL_FILE;
    };
}

namespace comphelper
{
    // consts
    const sal_Int32 UiEventsLogger_Impl::COLUMNS = 9;
    const OUString UiEventsLogger_Impl::CFG_ENABLED(RTL_CONSTASCII_USTRINGPARAM("EnablingAllowed"));
    const OUString UiEventsLogger_Impl::CFG_IDLETIMEOUT(RTL_CONSTASCII_USTRINGPARAM("IdleTimeout"));
    const OUString UiEventsLogger_Impl::CFG_LOGGING(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Logging"));
    const OUString UiEventsLogger_Impl::CFG_LOGPATH(RTL_CONSTASCII_USTRINGPARAM("LogPath"));
    const OUString UiEventsLogger_Impl::CFG_OOOIMPROVEMENT(RTL_CONSTASCII_USTRINGPARAM("OOoImprovement"));

    const OUString UiEventsLogger_Impl::CSSL_CSVFORMATTER(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.logging.CsvFormatter"));
    const OUString UiEventsLogger_Impl::CSSL_FILEHANDLER(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.logging.FileHandler"));
    const OUString UiEventsLogger_Impl::CSSL_LOGGERPOOL(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.logging.LoggerPool"));
    const OUString UiEventsLogger_Impl::CSSO_CORECONTROLLER(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.oooimprovement.CoreController"));
    const OUString UiEventsLogger_Impl::CSSU_PATHSUB(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.PathSubstitution"));

    const OUString UiEventsLogger_Impl::ETYPE_DISPATCH(RTL_CONSTASCII_USTRINGPARAM("dispatch"));
    const OUString UiEventsLogger_Impl::ETYPE_ROTATED(RTL_CONSTASCII_USTRINGPARAM("rotated"));
    const OUString UiEventsLogger_Impl::ETYPE_VCL(RTL_CONSTASCII_USTRINGPARAM("vcl"));

    const OUString UiEventsLogger_Impl::LOGGERNAME(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.oooimprovement.Core.UiEventsLogger"));
    const OUString UiEventsLogger_Impl::LOGORIGINWIDGET(RTL_CONSTASCII_USTRINGPARAM("comphelper.UiEventsLogger.LogOriginWidget"));
    const OUString UiEventsLogger_Impl::LOGORIGINAPP(RTL_CONSTASCII_USTRINGPARAM("comphelper.UiEventsLogger.LogOriginApp"));

    const OUString UiEventsLogger_Impl::UNKNOWN_ORIGIN(RTL_CONSTASCII_USTRINGPARAM("unknown origin"));
    const OUString UiEventsLogger_Impl::FN_CURRENTLOG(RTL_CONSTASCII_USTRINGPARAM("Current"));
    const OUString UiEventsLogger_Impl::FN_ROTATEDLOG(RTL_CONSTASCII_USTRINGPARAM("OOoImprove"));
    const OUString UiEventsLogger_Impl::LOGROTATE_EVENTNAME(RTL_CONSTASCII_USTRINGPARAM("onOOoImprovementLogRotated"));

    const OUString UiEventsLogger_Impl::URL_UNO(RTL_CONSTASCII_USTRINGPARAM(".uno:"));
    const OUString UiEventsLogger_Impl::URL_SPECIAL(RTL_CONSTASCII_USTRINGPARAM(".special:"));
    const OUString UiEventsLogger_Impl::URL_FILE(RTL_CONSTASCII_USTRINGPARAM("file:"));


    // public UiEventsLogger interface
    sal_Bool UiEventsLogger::isEnabled()
    {
        if ( UiEventsLogger_Impl::getEnabledFromCfg() )
        {
            try {
                UiEventsLogger_Impl::prepareMutex();
                Guard<Mutex> singleton_guard(UiEventsLogger_Impl::singleton_mutex);
                return UiEventsLogger_Impl::getInstance()->m_Active;
            } catch(...) { return false; } // never throws
        } // if ( )
        return sal_False;
    }

    sal_Int32 UiEventsLogger::getSessionLogEventCount()
    {
        try {
            UiEventsLogger_Impl::prepareMutex();
            Guard<Mutex> singleton_guard(UiEventsLogger_Impl::singleton_mutex);
            return UiEventsLogger_Impl::getInstance()->m_SessionLogEventCount;
        } catch(...) { return 0; } // never throws
    }

    void UiEventsLogger::appendDispatchOrigin(
        Sequence<PropertyValue>& args,
        const OUString& originapp,
        const OUString& originwidget)
    {
        sal_Int32 old_length = args.getLength();
        args.realloc(old_length+2);
        args[old_length].Name = UiEventsLogger_Impl::LOGORIGINAPP;
        args[old_length].Value = static_cast<Any>(originapp);
        args[old_length+1].Name = UiEventsLogger_Impl::LOGORIGINWIDGET;
        args[old_length+1].Value = static_cast<Any>(originwidget);
    }

    Sequence<PropertyValue> UiEventsLogger::purgeDispatchOrigin(
        const Sequence<PropertyValue>& args)
    {
        Sequence<PropertyValue> result(args.getLength());
        sal_Int32 target_idx=0;
        for(sal_Int32 source_idx=0; source_idx<args.getLength(); source_idx++)
            if(args[source_idx].Name != UiEventsLogger_Impl::LOGORIGINAPP
                && args[source_idx].Name != UiEventsLogger_Impl::LOGORIGINWIDGET)
                result[target_idx++] = args[source_idx];
        result.realloc(target_idx);
        return result;
    }

    void UiEventsLogger::logDispatch(
        const URL& url,
        const Sequence<PropertyValue>& args)
    {
        try {
            UiEventsLogger_Impl::prepareMutex();
            Guard<Mutex> singleton_guard(UiEventsLogger_Impl::singleton_mutex);
            UiEventsLogger_Impl::getInstance()->logDispatch(url, args);
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
            UiEventsLogger_Impl::prepareMutex();
            Guard<Mutex> singleton_guard(UiEventsLogger_Impl::singleton_mutex);
            UiEventsLogger_Impl::getInstance()->logVcl(parent_id, window_type, id, method, param);
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

    void UiEventsLogger::disposing()
    {
        // we dont want to create an instance just to dispose it
        UiEventsLogger_Impl::prepareMutex();
        Guard<Mutex> singleton_guard(UiEventsLogger_Impl::singleton_mutex);
        if(UiEventsLogger_Impl::instance!=UiEventsLogger_Impl::ptr())
            UiEventsLogger_Impl::getInstance()->disposing();
    }

    void UiEventsLogger::reinit()
    {
        UiEventsLogger_Impl::prepareMutex();
        Guard<Mutex> singleton_guard(UiEventsLogger_Impl::singleton_mutex);
        if(UiEventsLogger_Impl::instance)
        {
            UiEventsLogger_Impl::instance->disposing();
            delete UiEventsLogger_Impl::instance;
            UiEventsLogger_Impl::instance = NULL;
        }
    }

    // private UiEventsLogger_Impl methods
    UiEventsLogger_Impl::UiEventsLogger_Impl()
        : m_Active(UiEventsLogger_Impl::shouldActivate())
        , m_LogPath(UiEventsLogger_Impl::getLogPathFromCfg())
        , m_IdleTimeout(UiEventsLogger_Impl::getIdleTimeoutFromCfg())
        , m_SessionLogEventCount(0)
    {
        lcl_SetupOriginAppAbbr(m_OriginAppAbbr);
        lcl_SetupOriginWidgetAbbr(m_OriginWidgetAbbr);
        m_LastLogEventTime.Seconds = m_LastLogEventTime.Nanosec = 0;
        if(m_Active) rotate();
        if(m_Active) initializeLogger();
    }

    void UiEventsLogger_Impl::logDispatch(
        const URL& url,
        const Sequence<PropertyValue>& args)
    {
        if(!m_Active) return;
        if(!url.Complete.match(URL_UNO)
            && !url.Complete.match(URL_FILE)
            && !url.Complete.match(URL_SPECIAL))
        {
            return;
        }
        checkIdleTimeout();

        Sequence<OUString> logdata = Sequence<OUString>(COLUMNS);
        logdata[0] = ETYPE_DISPATCH;
        sal_Int32 originapp_idx = findIdx(args, LOGORIGINAPP);
        if(originapp_idx!=-1)
        {
            OUString app;
            args[originapp_idx].Value >>= app;
            map<OUString, OUString>::iterator abbr_it = m_OriginAppAbbr.find(app);
            if(abbr_it != m_OriginAppAbbr.end())
                app = abbr_it->second;
            logdata[1] = app;
        }
        else
            logdata[1] = UNKNOWN_ORIGIN;
        sal_Int32 originwidget_idx = findIdx(args, LOGORIGINWIDGET);
        if(originwidget_idx!=-1)
        {
            OUString widget;
            args[originwidget_idx].Value >>= widget;
            map<OUString, OUString>::iterator widget_it = m_OriginWidgetAbbr.find(widget);
            if(widget_it != m_OriginWidgetAbbr.end())
                widget = widget_it->second;
            logdata[2] = widget;
        }
        else
            logdata[2] = UNKNOWN_ORIGIN;
        if(url.Complete.match(URL_FILE))
            logdata[3] = URL_FILE;
        else
            logdata[3] = url.Main;
        OSL_TRACE("UiEventsLogger Logging: %s,%s,%s,%s,%s,%s,%s,%s",
            OUStringToOString(logdata[0],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[1],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[2],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[3],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[4],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[5],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[6],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[7],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[8],RTL_TEXTENCODING_UTF8).getStr());
        m_Logger->log(LogLevel::INFO, m_Formatter->formatMultiColumn(logdata));
        m_SessionLogEventCount++;
    }

    void UiEventsLogger_Impl::logRotated()
    {
        Sequence<OUString> logdata = Sequence<OUString>(COLUMNS);
        logdata[0] = ETYPE_ROTATED;
        OSL_TRACE("UiEventsLogger Logging: %s,%s,%s,%s,%s,%s,%s,%s",
            OUStringToOString(logdata[0],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[1],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[2],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[3],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[4],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[5],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[6],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[7],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[8],RTL_TEXTENCODING_UTF8).getStr());
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
        checkIdleTimeout();

        OUStringBuffer buf;
        Sequence<OUString> logdata = Sequence<OUString>(COLUMNS);
        logdata[0] = ETYPE_VCL;
        logdata[4] = parent_id;
        logdata[5] = buf.append(window_type).makeStringAndClear();
        logdata[6] = id;
        logdata[7] = method;
        logdata[8] = param;
        OSL_TRACE("UiEventsLogger Logging: %s,%s,%s,%s,%s,%s,%s,%s",
            OUStringToOString(logdata[0],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[1],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[2],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[3],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[4],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[5],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[6],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[7],RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(logdata[8],RTL_TEXTENCODING_UTF8).getStr());
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

        // getting the Core Uno proxy object
        // It will call disposing and make sure we clear all our references
        {
            Reference<XTerminateListener> xCore(
                sm->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.oooimprovement.Core"))),
                UNO_QUERY);
            Reference<XDesktop> xDesktop(
                sm->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))),
                UNO_QUERY);
            if(!(xCore.is() && xDesktop.is()))
            {
                m_Active = false;
                return;
            }
            xDesktop->addTerminateListener(xCore);
        }
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
            columns[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("eventtype"));
            columns[1] = OUString(RTL_CONSTASCII_USTRINGPARAM("originapp"));
            columns[2] = OUString(RTL_CONSTASCII_USTRINGPARAM("originwidget"));
            columns[3] = OUString(RTL_CONSTASCII_USTRINGPARAM("uno url"));
            columns[4] = OUString(RTL_CONSTASCII_USTRINGPARAM("parent id"));
            columns[5] = OUString(RTL_CONSTASCII_USTRINGPARAM("window type"));
            columns[6] = OUString(RTL_CONSTASCII_USTRINGPARAM("id"));
            columns[7] = OUString(RTL_CONSTASCII_USTRINGPARAM("method"));
            columns[8] = OUString(RTL_CONSTASCII_USTRINGPARAM("parameter"));
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
            sm->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.oooimprovement.CoreController"))),
            UNO_QUERY);
        if(!core_c.is()) return false;
        return core_c->enablingUiEventsLoggerAllowed(1);
    }

    UiEventsLogger_Impl::ptr UiEventsLogger_Impl::instance = UiEventsLogger_Impl::ptr();
    UiEventsLogger_Impl::ptr UiEventsLogger_Impl::getInstance()
    {
        if(instance == NULL)
            instance = UiEventsLogger_Impl::ptr(new UiEventsLogger_Impl());
        return instance;
    }

    Mutex * UiEventsLogger_Impl::singleton_mutex = NULL;
    void UiEventsLogger_Impl::prepareMutex()
    {
        if(singleton_mutex == NULL)
        {
            Guard<Mutex> global_guard(Mutex::getGlobalMutex());
            if(singleton_mutex == NULL)
                singleton_mutex = new Mutex();
        }
    }

    sal_Int32 UiEventsLogger_Impl::findIdx(const Sequence<PropertyValue>& args, const OUString& key)
    {
        for(sal_Int32 i=0; i<args.getLength(); i++)
            if(args[i].Name == key)
                return i;
        return -1;
    }

    void UiEventsLogger_Impl::disposing()
    {
        m_Active = false;
        m_Logger.clear() ;
        m_LogHandler.clear();
        m_Formatter.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
