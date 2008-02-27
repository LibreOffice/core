/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: signal.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:27:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppunit.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (defined UNX) || (defined OS2)
#include <signal.h>
#include <errno.h>
#endif

#include <fstream>
#include <vector>
#include <hash_map>

#include <rtl/tres.h>
#include <rtl/string.hxx>

#include "testshl/autoregisterhelper.hxx"
#include "testshl/getopt.hxx"
#include "signal.hxx"
#include <cppunit/tagvalues.hxx>
#include <unistd.h>
#include "testshl/filehelper.hxx"
#include <cppunit/result/TestResult.h>
#include "cppunit/signaltest.h"
#include "cppunit/Exception.h"

#ifdef WNT
#include "testshl/winstuff.hxx"
#endif

// typedef std::vector<std::string> StringList;
// StringList sCurrentNodeName;
CppUnit::TestResult *pTestResult = NULL;
std::string sSignalFile;

typedef std::hash_map< std::string, int > HashMap;
HashMap m_aSignalHash;
bool bSignalsCached = false;
bool bDoNotTouchSignalFile = false;

// -----------------------------------------------------------------------------

// return 'true' if signalfile doesn't exist.
// else 'false'
bool existsSignalFile(std::string const& _sSignalFilename)
{
    FILE* pFile = fopen(_sSignalFilename.c_str(), "r");
    if (!pFile)
    {
        return false;
    }
    fprintf(stderr, "'%s' exists.\n", _sSignalFilename.c_str());
    fclose(pFile);
    return true;
}

// -----------------------------------------------------------------------------
void createEmptySignalFile(std::string const& _sSignalFilename)
{
    FILE* pFile = fopen(_sSignalFilename.c_str(), "w");
    if (!pFile)
    {
        fprintf(stderr, "error: Could not create signal helper file %s for signal info.\n", _sSignalFilename.c_str());
    }
    else
    {
        fprintf(pFile, "# This is an auto generated helper file for signal handling.\n");
        fprintf(pFile, "# An entry start by '#' is a comment.\n");
        fprintf(pFile, "# All other are test functions which have abort, before this line is removed.\n");
        fprintf(pFile, "# So you have to check this functions by hand.\n");

        fclose(pFile);
    }
}

// -----------------------------------------------------------------------------
/** get Current PID.
*/
inline ::rtl::OUString getCurrentPID(  )
{
        //~ Get current PID and turn it into OUString;
        sal_uInt32 nPID = 0;
#ifdef WNT
        nPID = WinGetCurrentProcessId();
#else
        nPID = getpid();
#endif
        return ( ::rtl::OUString::valueOf( static_cast<long>(nPID ) ) );
}
// -----------------------------------------------------------------------------
static std::string integerToAscii(sal_uInt32 nValue)
{
    sal_Char cBuf[30];
    sal_Char *pBuf = cBuf;
    sprintf(pBuf, "%d", static_cast<unsigned int>(nValue));
    return std::string(pBuf);
}
void my_sleep(int sec);

// -----------------------------------------------------------------------------
void setSignalFilename(GetOpt & opt)
{
    if (opt.hasOpt("-dntsf") || opt.hasOpt("-donottouchsignalfile"))
    {
        // special feature, for debugging, so the signal file will not manipulate.
        // but create, if no one exist.
        bDoNotTouchSignalFile = true;
    }

    if (opt.hasOpt("-sf") || opt.hasOpt("-signalfile"))
    {
        if (opt.hasOpt("-sf"))
        {
            sSignalFile = opt.getOpt("-sf");
        }
        else if (opt.hasOpt("-signalfile"))
        {
            sSignalFile = opt.getOpt("-signalfile");
        }
    }
    else
    {
        std::string sPath;
        // std::string sPath(FileHelper::getTempPath());
        std::string sFilename("signalfile");
        std::string sFilenameExt(".txt");
        bool bCanQuitLoop = true;
        do
        {

// #ifdef WNT
//         sPath += "\\";
// #endif
// #ifdef UNX
//         sPath += "/";
// #endif
            sPath = sFilename;
            // BUG: i72675
            // add "_12345" where 12345 is the current process ID

            TimeValue aTimeValue;
            osl_getSystemTime(&aTimeValue);

            sPath += "_";
            sPath += integerToAscii(aTimeValue.Seconds);
            // rtl::OUString suPID = getCurrentPID();
            // rtl::OString sPID = rtl::OUStringToOString(suPID, RTL_TEXTENCODING_ASCII_US);
            // sPath += sPID.getStr();
            sPath += sFilenameExt;
            bCanQuitLoop = true;
            if (existsSignalFile(sPath))
            {
                // there is already a signal file, wait a second, choose an other one.
                my_sleep(1);
                bCanQuitLoop = false;
            }
        }
        while (!(bCanQuitLoop));

        sSignalFile = sPath;
        fprintf(stderr, "Use default signal file name '%s'\n", sSignalFile.c_str());
    }

    if (opt.hasOpt("-dnrmsf"))
    {
        fprintf(stderr, "'Don't remove signal file' (-dnrmsf) is set.\n");
    }
    else
    {
        if (bDoNotTouchSignalFile == true)
        {
            fprintf(stderr, "warning: 'Don't touch signal file' parameter (-dntsf) is set, will not remove existing signal file.\n");
        }
        else
        {
            // remove signalfile
            createEmptySignalFile(sSignalFile);
        }
    }
}

// -----------------------------------------------------------------------------
bool doNotTouchSignalFile() { return bDoNotTouchSignalFile; }

// -----------------------------------------------------------------------------
std::string buildTestFunctionName(std::string const& _sName)
{
    std::string sName;
    if (pTestResult)
    {
        sName = pTestResult->getNodeName();
        sName += ".";
    }

/*
    for (StringList::const_iterator it = sCurrentNodeName.begin();
         it != sCurrentNodeName.end();
         ++it)
    {
        sName += *it;
        sName += ".";
    }
*/
    sName += _sName;

    return sName;
}
// -----------------------------------------------------------------------------
// old: void executionPushName(std::string const& _sName)
// old: {
// old:     sCurrentNodeName.push_back(_sName);
// old: }
// old: void executionPopName()
// old: {
// old:     sCurrentNodeName.pop_back();
// old: }
// old:

// -----------------------------------------------------------------------------
// ------------------------------ Signal Handling ------------------------------
// -----------------------------------------------------------------------------

// std::string sLastTestFunctionName;

std::string getSignalName(sal_Int32 nSignalNo);
// -----------------------------------------------------------------------------

std::string getSignalFilename()
{
    return sSignalFile;
}

// -----------------------------------------------------------------------------
// void storeNoSignal(std::string const& _sTestName)
// {
    // sLastTestFunctionName = buildTestFunctionName(_sTestName);
    // std::ofstream out(getSignalFilename().c_str(), std::ios::out);
    // out << NO_SIGNAL << std::endl;               // no signal!
// }

void markSignalAsAlreadyDone(sal_Int32 _nSignalNo)
{
    // std::ofstream out(getSignalFilename().c_str(), std::ios::out | std::ios::app);
    FILE *out = fopen(getSignalFilename().c_str(), "a");
    if (out != NULL)
    {
//#         out << "# the previous test function creates signal: "
//#             << getSignalName(_nSignalNo)
//#             << " ("
//#             << _nSignalNo
//#             << ")" << std::endl;
//#         // out << sLastTestFunctionName << std::endl;               // SIGNAL!

        fprintf(out, "# the previous test function creates signal: %s(%d)\n", getSignalName(_nSignalNo).c_str(), SAL_STATIC_CAST(int, _nSignalNo));
        // fprintf(out, "%s\n", sLastTestFunctionName );
        fclose(out);
    }
    else
    {
        fprintf(stderr, "error: Can't write signal info to file %s \n", getSignalFilename().c_str());
    }
}

// -----------------------------------------------------------------------------

Signal hasSignaled(std::string const& _sTestName)
{
    // BACK:  true: signal
    //       false: nothing

    if (bSignalsCached == true)
    {

        if (m_aSignalHash.find(buildTestFunctionName(_sTestName)) != m_aSignalHash.end())
        {
            return HAS_SIGNAL;
        }
        return NO_SIGNAL;
    }

    std::ifstream in(getSignalFilename().c_str(), std::ios::in);

    // std::cout << "Check for signal" << std::endl;
    std::string sLine, sLastLine;
    while (std::getline(in, sLine))
    {
        // std::cout << sTest << std::endl;
        char ch = sLine[0];
        if (isspace(ch) == 0 &&
            sLine.size() > 0)
        {
            if (ch == '#')
            {
                if (sLastLine.size() > 0)
                {
                    rtl::OString aStrLine(sLine.c_str());
                    sal_Int32 nIdx = aStrLine.indexOf("(") + 1;
                    sal_Int32 nIdx2 = aStrLine.indexOf(")");
                    sal_Int32 nSignalNo = 0;
                    if (nIdx > 0 && nIdx2 > 0)
                    {
                        rtl::OString sSignalNo = aStrLine.copy(nIdx, nIdx2 - nIdx);
                        nSignalNo = sSignalNo.toInt32();
                        m_aSignalHash[sLastLine] = nSignalNo;
                    }
                    sLastLine.clear();
                }
            }
            else
            {
                // if (sTest == buildTestFunctionName(_sTestName))
                m_aSignalHash[sLine] = 1;
                sLastLine = sLine;
                // return HAS_SIGNAL;
            }
        }
    }

    bSignalsCached = true;
    return hasSignaled(_sTestName);
    // return NO_SIGNAL;
}

#ifdef UNX

// -----------------------------------------------------------------------------

void release_signal_Handling();

//# void signalFunction(int value)
//# {
//#     std::cout << "Signal caught: (" << value << "), please restart." << std::endl;
//#     markSignalAsAlreadyDone();
//#
//#     release_signal_Handling();
//#         std::cout.flush();
//#     abort();
//# }

// -----------------------------------------------------------------------------
extern "C" void SignalHandlerFunction(int _nSignalNo, siginfo_t *, void*)
{
    // std::cout << "Signal caught: " << getSignalName(_nSignalNo) << " (" << _nSignalNo << "), please restart." << std::endl;
    fprintf(stderr, "Signal caught %s(%d)\n", getSignalName(_nSignalNo).c_str(), _nSignalNo);
    markSignalAsAlreadyDone(_nSignalNo);

    release_signal_Handling();
    // std::cout.flush();
    abort();
}

// -----------------------------------------------------------------------------
// This is a copy of the osl/signal.c code
#define ACT_IGNORE      1
#define ACT_ABORT       2
#define ACT_EXIT        3
#define ACT_SYSTEM      4
#define ACT_HIDE        5

extern "C" {
static struct SignalAction
{
    int Signal;
    int Action;
    void (*Handler)(int);
} Signals[] =
{
    { SIGHUP,    ACT_IGNORE, NULL },    /* hangup */
    { SIGINT,    ACT_EXIT,   NULL },    /* interrupt (rubout) */
    { SIGQUIT,   ACT_ABORT,  NULL },    /* quit (ASCII FS) */
    { SIGILL,    ACT_SYSTEM,  NULL },    /* illegal instruction (not reset when caught) */
/* changed from ACT_ABOUT to ACT_SYSTEM to try and get collector to run*/
    { SIGTRAP,   ACT_ABORT,  NULL },    /* trace trap (not reset when caught) */
#if ( SIGIOT != SIGABRT )
    { SIGIOT,    ACT_ABORT,  NULL },    /* IOT instruction */
#endif
//      { SIGABRT,   ACT_ABORT,  NULL },    /* used by abort, replace SIGIOT in the future */
#ifdef SIGEMT
    { SIGEMT,    ACT_SYSTEM,  NULL },    /* EMT instruction */
/* changed from ACT_ABORT to ACT_SYSTEM to remove handler*/
/* SIGEMT may also be used by the profiler - so it is probably not a good
   plan to have the new handler use this signal*/
#endif
    { SIGFPE,    ACT_ABORT,  NULL },    /* floating point exception */
    { SIGKILL,   ACT_SYSTEM, NULL },    /* kill (cannot be caught or ignored) */
    { SIGBUS,    ACT_ABORT,  NULL },    /* bus error */
    { SIGSEGV,   ACT_ABORT,  NULL },    /* segmentation violation */
#ifdef SIGSYS
    { SIGSYS,    ACT_ABORT,  NULL },    /* bad argument to system call */
#endif
    { SIGPIPE,   ACT_HIDE,   NULL },    /* write on a pipe with no one to read it */
    { SIGALRM,   ACT_EXIT,   NULL },    /* alarm clock */
    { SIGTERM,   ACT_EXIT,   NULL },    /* software termination signal from kill */
    { SIGUSR1,   ACT_SYSTEM, NULL },    /* user defined signal 1 */
    { SIGUSR2,   ACT_SYSTEM, NULL },    /* user defined signal 2 */
    { SIGCHLD,   ACT_SYSTEM, NULL },    /* child status change */
#ifdef SIGPWR
    { SIGPWR,    ACT_IGNORE, NULL },    /* power-fail restart */
#endif
    { SIGWINCH,  ACT_IGNORE, NULL },    /* window size change */
    { SIGURG,    ACT_EXIT,   NULL },    /* urgent socket condition */
#ifdef SIGPOLL
    { SIGPOLL,   ACT_EXIT,   NULL },    /* pollable event occured */
#endif
    { SIGSTOP,   ACT_SYSTEM, NULL },    /* stop (cannot be caught or ignored) */
    { SIGTSTP,   ACT_SYSTEM, NULL },    /* user stop requested from tty */
    { SIGCONT,   ACT_SYSTEM, NULL },    /* stopped process has been continued */
    { SIGTTIN,   ACT_SYSTEM, NULL },    /* background tty read attempted */
    { SIGTTOU,   ACT_SYSTEM, NULL },    /* background tty write attempted */
    { SIGVTALRM, ACT_EXIT,   NULL },    /* virtual timer expired */
    { SIGPROF,   ACT_SYSTEM,   NULL },    /* profiling timer expired */
/*Change from ACT_EXIT to ACT_SYSTEM for SIGPROF is so that profiling signals do
  not get taken by the new handler - the new handler does not pass on context
  information which causes 'collect' to crash. This is a way of avoiding
  what looks like a bug in the new handler*/
    { SIGXCPU,   ACT_ABORT,  NULL },    /* exceeded cpu limit */
    { SIGXFSZ,   ACT_ABORT,  NULL }     /* exceeded file size limit */
};
}

const int NoSignals = sizeof(Signals) / sizeof(struct SignalAction);

#endif /* UNX */

// -----------------------------------------------------------------------------
void init_signal_Handling(CppUnit::TestResult *_pResult)
{
    pTestResult = _pResult;
#ifdef UNX

//    signal(SIGSEGV, signalFunction);
    // signal(SIGSEGV, signalFunction);
    // signal(SIGFPE, signalFunction);

//    signal(1, signalFunction);
    // struct sigaction action, oldaction;
    // action.sa_sigaction = signalFunction2;
    // action.sa_flags     = SA_ONESHOT /* | SA_SIGINFO */;

    struct sigaction act;
    struct sigaction oact;

    // act.sa_handler = SignalHandlerFunction;
    act.sa_flags     = SA_RESTART;
    // act.sa_flags     = SA_ONESHOT /* | SA_SIGINFO */;
    act.sa_sigaction = SignalHandlerFunction;

    sigfillset(&(act.sa_mask));

    /* Initialize the rest of the signals */
    for (int i = 0; i < NoSignals; i++)
    {
        if (Signals[i].Action != ACT_SYSTEM)
        {
            if (Signals[i].Action == ACT_HIDE)
            {
                struct sigaction ign;

                ign.sa_handler = SIG_IGN;
                ign.sa_flags   = 0;
                sigemptyset(&ign.sa_mask);

                if (sigaction(Signals[i].Signal, &ign, &oact) == 0)
                    Signals[i].Handler = oact.sa_handler;
                else
                    Signals[i].Handler = SIG_DFL;
            }
            else
                if (sigaction(Signals[i].Signal, &act, &oact) == 0)
                    Signals[i].Handler = oact.sa_handler;
                else
                    Signals[i].Handler = SIG_DFL;
        }
    }
#endif

    // ------------ signal helper file must exist -----------------
    FILE* pFile = fopen(getSignalFilename().c_str(), "r");
    if (!pFile)
    {
        createEmptySignalFile( getSignalFilename() );
    }
    else
    {
        fclose(pFile);
    }
}

// -----------------------------------------------------------------------------
void release_signal_Handling()
{
    // frees all signals
#ifdef UNX
    int i;
    struct sigaction act;

    act.sa_flags   = 0;
    sigemptyset(&(act.sa_mask));

    /* Initialize the rest of the signals */
    for (i = NoSignals - 1; i >= 0; i--)
    {
        if (Signals[i].Action != ACT_SYSTEM)
        {
            act.sa_handler = Signals[i].Handler;

            sigaction(Signals[i].Signal, &act, NULL);
        }
    }
#endif
}

// -----------------------------------------------------------------------------
Signal signalCheck(CppUnit::TestResult* _pResult, std::string const& _sTestName)
{
    // BACK: HAS_SIGNAL: the test has already done and signaled
    if (hasSignaled(_sTestName) == HAS_SIGNAL)
    {
        // std::cout << "The Test '" << buildTestFunctionName(_sTestName) << "' is marked as signaled." << std::endl;
        std::string sTestFunctionName = buildTestFunctionName(_sTestName);
        fprintf(stderr, "The Test '%s' is marked as signaled.\n", sTestFunctionName.c_str());
        if (_pResult)
        {
            CppUnit::SignalTest *pTest = new CppUnit::SignalTest(_sTestName);

            std::string sErrorText = "Function is marked as signaled: ";
            sal_Int32 nSignalNo = m_aSignalHash[sTestFunctionName];
            sErrorText += getSignalName(nSignalNo);
            sErrorText += " (";
            sErrorText += OptionHelper::integerToAscii(nSignalNo);
            sErrorText += ")";

            _pResult->addError(pTest, new CppUnit::SignalException(sErrorText), ErrorType::ET_SIGNAL);
        }
        return HAS_SIGNAL;
    }

    // storeNoSignal(_sTestName);
    return NO_SIGNAL;
}

// -----------------------------------------------------------------------------
bool copyFile(std::string const& _sFrom, std::string const& _sTo)
{
    bool bRetValue = false;
    const int MAXBUFSIZE = 1024;
    char buff[MAXBUFSIZE];
    FILE *in = fopen(_sFrom.c_str(), "r");
    if (in == NULL)
    {
        fprintf(stderr, "error: Can't open file %s for read to copy.\n", _sFrom.c_str());
        bRetValue = false;
    }
    else
    {
        FILE *out = fopen(_sTo.c_str(), "w");
        if (out == NULL)
        {
            fclose(in);
            fprintf(stderr, "error: Can't open file %s for write to copy.\n", _sTo.c_str());
            bRetValue = false;
        }
        else
        {
            int nRealGot = 0;
            while(!feof(in))
            {
                nRealGot = fread(buff, sizeof(char), MAXBUFSIZE, in);
                if (nRealGot > 0)
                {
                    fwrite(buff, sizeof(char), nRealGot, out);
                }
            }
            bRetValue = true;
            fclose(out);
            fclose(in);
        }
    }
    return bRetValue;
}

// -----------------------------------------------------------------------------
void signalStartTest(std::string const& _sName)
{
    if (doNotTouchSignalFile()) return;

    // fprintf(stderr, "### signalStartTest!\n");
    // due to the fact, that functions are vicious, we write the name first.
    // if it isn't vivious, than we removed it.
    std::string sNewName = getSignalFilename();
    sNewName += ".bak";
    if (copyFile(getSignalFilename(), sNewName))
    {
        // std::ofstream out(getSignalFilename().c_str(), std::ios::out | std::ios::app);
        FILE *out = fopen(getSignalFilename().c_str(), "a");
        if (out != NULL)
        {
            // out << buildTestFunctionName(_sName) << std::endl;
            fprintf(out, "%s\n", buildTestFunctionName(_sName).c_str());
            fclose(out);
        }
        else
        {
            fprintf(stderr, "error: Can't open file %s for append.\n", getSignalFilename().c_str());
        }
    }
    else
    {
        fprintf(stderr, "error: Can't copy signal helper from file %s to file %s, %d\n", getSignalFilename().c_str(), sNewName.c_str(), errno);
    }
}

// -----------------------------------------------------------------------------
void signalEndTest()
{
    if (doNotTouchSignalFile()) return;

    // fprintf(stderr, "### signalEndTest!\n");
    if (0 != remove(getSignalFilename().c_str()))
    {
        fprintf(stderr, "error: Can't delete file %s\n", getSignalFilename().c_str());
    }
    else
    {
        std::string sNewName = getSignalFilename();
        sNewName += ".bak";
        if (0 != rename(sNewName.c_str(), getSignalFilename().c_str()))
        {
            fprintf(stderr, "error: Can't rename file %s to file %s errno: %d\n", sNewName.c_str(), getSignalFilename().c_str(), errno);
        }
    }
}

// -----------------------------------------------------------------------------
void removeSignalFile(GetOpt & opt)
{
    // fprintf(stderr, "### remove signal file: '%s'\n", sSignalFile.c_str());
    if (opt.hasOpt("-dnrmsf"))
    {
        return;
    }
    if (bDoNotTouchSignalFile == true)
    {
        return;
    }
    remove(getSignalFilename().c_str());
}

// -----------------------------------------------------------------------------

sal_Int32 SignalHandlerA( TagHelper const& _aTagItems )
{
    sal_Int32 nRetValue = 0;
    TagData nTagType     = _aTagItems.GetTagData(TAG_TYPE, 0);
// LLA: unused
//    hTestResult hResult  = (hTestResult) _aTagItems.GetTagData(TAG_RESULT_PTR, 0 /* NULL */ );
//    CppUnit::TestResult* pResult = (CppUnit::TestResult*)hResult;

    try
    {
        switch(nTagType)
        {
        // old: case SIGNAL_PUSH_NAME:
        // old: {
        // old:     const char* sName = (const char*) _aTagItems.GetTagData(TAG_NODENAME);
        // old:     if (sName != NULL)
        // old:         signalPushName(sName);
        // old:     break;
        // old: }
        // old:
        // old: case SIGNAL_POP_NAME:
        // old:     signalPopName();
        // old:     break;

        // old: case SIGNAL_CHECK:
        // old: {
        // old:     const char* sName = (const char*) _aTagItems.GetTagData(TAG_NODENAME);
        // old:     if (sName != NULL)
        // old:     {
        // old:         nRetValue = signalCheck(sName);
        // old:     }
        // old:     break;
        // old: }

        // old: case INIT_SIGNAL_HANDLING:
        // old:     init_signal_Handling();
        // old:     break;
        // old:
        // old: case RELEASE_SIGNAL_HANDLING:
        // old:     release_signal_Handling();
        // old:     break;

        case SIGNAL_START_TEST:
        {
            // fprintf(stderr, "### SIGNAL_START_TEST!\n");
            const char* sName = (const char*) _aTagItems.GetTagData(TAG_NODENAME);
            if (sName != NULL)
            {
                signalStartTest(sName);
            }
            break;
        }

        case SIGNAL_END_TEST:
        {
            // fprintf(stderr, "### SIGNAL_END_TEST!\n");
            const char* sName = (const char*) _aTagItems.GetTagData(TAG_NODENAME);
            if (sName != NULL)
            {
                signalEndTest();
            }
            break;
        }

        default:
            fprintf(stderr, "error: SignalHandlerA: Can't handle the tag type %d\n", SAL_STATIC_CAST(int, nTagType));
            fflush(stderr);
            // throw std::exception(/*std::string("Unknown TYPE_TAG Exception.")*/);
        }
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "error: SignalHandlerA: Exception caught: %s\n", e.what());
        fflush(stderr);
        // throw e;
    }

    return nRetValue;
}
// -----------------------------------------------------------------------------
// This a little bit more abstract code, could be easier to modify or expand.

sal_Int32 CheckExecution(CppUnit::TestResult* _pResult, std::string const& _sName)
{
    // more checks in the corresponding job lists
    if (_pResult)
    {
        if (! _pResult->isAllowedToExecute(_sName))
        {
            return DO_NOT_EXECUTE;
        }
    }

    // Check if the given test should be executed.
    if (signalCheck(_pResult, _sName) == HAS_SIGNAL)
    {
        return DO_NOT_EXECUTE;
    }

    return GO_EXECUTE;
}

// -----------------------------------------------------------------------------
sal_Int32 ExecutionA( TagHelper const& _aTagItems )
{
    sal_Int32 nRetValue = 0;
    TagData nTagType     = _aTagItems.GetTagData(TAG_TYPE, 0);
    hTestResult hResult  = (hTestResult) _aTagItems.GetTagData(TAG_RESULT_PTR, 0 /* NULL */ );
    CppUnit::TestResult* pResult = (CppUnit::TestResult*)hResult;

    try
    {
        switch(nTagType)
        {
        case EXECUTION_CHECK:
        {
            const char* sName = (const char*) _aTagItems.GetTagData(TAG_NODENAME);
            if (sName)
            {
                nRetValue = CheckExecution(pResult, sName);
                if (nRetValue == GO_EXECUTE)
                {
                    if (pResult && pResult->isOptionWhereAmI())
                    {
                        printf("# This is: %s\n", buildTestFunctionName(sName).c_str());
                    }
                }
            }

            break;
        }

        // old: case EXECUTION_PUSH_NAME:
        // old: {
        // old:     const char* sName = (const char*) _aTagItems.GetTagData(TAG_NODENAME);
        // old:     if (sName != NULL)
        // old:         executionPushName(sName);
        // old:     break;
        // old: }
        // old:
        // old: case EXECUTION_POP_NAME:
        // old:     executionPopName();
        // old:     break;

        case INIT_TEST:
            init_signal_Handling(pResult);
            break;

        case RELEASE_TEST:
            release_signal_Handling();
            break;

        default:
            fprintf(stderr, "ExceptionA: Can't handle the tag type %d\n", SAL_STATIC_CAST(int, nTagType));
            break;
        }
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "ExecutionA: exception caught: %s\n", e.what());
        fflush(stderr);
        // throw e;
    }
    return nRetValue;
}

// -----------------------------------------------------------------------------
std::string getSignalName(sal_Int32 nSignalNo)
{
    std::string sValue;
#ifdef UNX
    switch(nSignalNo)
    {
    case SIGHUP:
        sValue = "SIGHUP";
        break;

    case SIGINT:
        sValue = "SIGINT";
        break;

    case SIGQUIT:
        sValue = "SIGQUIT";
        break;

    case SIGILL:
        sValue = "SIGILL";
        break;

    case SIGTRAP:
        sValue = "SIGTRAP";
        break;

#if ( SIGIOT != SIGABRT )
    case SIGIOT:
        sValue = "SIGIOT";
        break;
#endif
//      case SIGABRT:
#ifdef SIGEMT
    case SIGEMT:
        sValue = "SIGEMT";
        break;
#endif
    case SIGFPE:
        sValue = "SIGFPE";
        break;

    case SIGKILL:
        sValue = "SIGKILL";
        break;

    case SIGBUS:
        sValue = "SIGBUS";
        break;

    case SIGSEGV:
        sValue = "SIGSEGV";
        break;

#ifdef SIGSYS
    case SIGSYS:
        sValue = "SIGSYS";
        break;
#endif
    case SIGPIPE:
        sValue = "SIGPIPE";
        break;

    case SIGALRM:
        sValue = "SIGALRM";
        break;

    case SIGTERM:
        sValue = "SIGTERM";
        break;

    case SIGUSR1:
        sValue = "SIGUSR1";
        break;

    case SIGUSR2:
        sValue = "SIGUSR2";
        break;

    case SIGCHLD:
        sValue = "SIGCHLD";
        break;

#ifdef SIGPWR
    case SIGPWR:
        sValue = "SIGPWR";
        break;
#endif
    case SIGWINCH:
        sValue = "SIGWINCH";
        break;

    case SIGURG:
        sValue = "SIGURG";
        break;

#ifdef SIGPOLL
    case SIGPOLL:
        sValue = "SIGPOLL";
        break;
#endif
    case SIGSTOP:
        sValue = "SIGSTOP";
        break;

    case SIGTSTP:
        sValue = "SIGTSTP";
        break;

    case SIGCONT:
        sValue = "SIGCONT";
        break;

    case SIGTTIN:
        sValue = "SIGTTIN";
        break;

    case SIGTTOU:
        sValue = "SIGTTOU";
        break;

    case SIGVTALRM:
        sValue = "SIGVTALRM";
        break;

    case SIGPROF:
        sValue = "SIGPROF";
        break;

    case SIGXCPU:
        sValue = "SIGXCPU";
        break;

    case SIGXFSZ:
        sValue = "SIGXFSZ";
        break;

    default:
        sValue = "Unhandled Signal.";
    }
#else
    (void) nSignalNo; // unused
#endif
    return sValue;
}

// The following sets variables for GNU EMACS
// Local Variables:
// tab-width:4
// End:
