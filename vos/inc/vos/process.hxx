/*************************************************************************
 *
 *  $RCSfile: process.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VOS_PROCESS_HXX_
#define _VOS_PROCESS_HXX_

#ifndef _RTL_USTRING_
#   include <rtl/ustring>
#endif
#ifndef _VOS_MUTEX_HXX_
#   include <vos/mutex.hxx>
#endif
#ifndef _VOS_SECURITY_HXX_
#   include <vos/security.hxx>
#endif
#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif
#ifndef _VOS_SOCKET_HXX_
#   include <vos/socket.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#   include <osl/process.h>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif

class OProcess;

/** helper class to fill a vector of command line arguments
 */
class OArgumentList
{
    sal_uInt32 n_Args;
    rtl_uString** m_aVec;

public:

    OArgumentList();
    OArgumentList( sal_uInt32 nArgs, const NAMESPACE_RTL(OUString)* aArgument1, ... );
    // switched argument list to avoid ambiguity with previous constructor.
    OArgumentList( const NAMESPACE_RTL(OUString) aArgumentList[], sal_uInt32 nArgs );

    OArgumentList( const OArgumentList& rOther);

    OArgumentList& operator=( const OArgumentList& rOther);

    virtual ~OArgumentList();

    friend class OProcess;
};

/** helper class to fill a vector of environment settings
 */
class OEnvironment
{
    sal_Int32 n_Vars;
    rtl_uString** m_aVec;

public:

    OEnvironment();
    OEnvironment( sal_uInt32 nVars, const NAMESPACE_RTL(OUString)* aVariable1, ... );
    // switched argument list to avoid ambiguity with previous constructor.
    OEnvironment( const NAMESPACE_RTL(OUString) aVariableList[], sal_uInt32 nVars );

    OEnvironment( const OEnvironment& rOther );

    OEnvironment& operator=( const OEnvironment& rOther );

    virtual ~OEnvironment();

    friend class OProcess;
};


/** startup child processes.
    @see OStartupInfo
    Used for starting an monitoring child processes with special features:
    <ul><li>setting enviroments,
    <li>setting working directories,
    <li>setting user rights and security,
    <li>providing ioresources like file descriptors and sockets.</ul>
*/
class OProcess : public OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OProcess, vos));

public:

    /** Options for execution mode:
    */
    enum TProcessOption
    {
        TOption_Wait       = osl_Process_WAIT,       // wait for completion
        TOption_SearchPath = osl_Process_SEARCHPATH, // search path for executable
        TOption_Detached   = osl_Process_DETACHED,   // run detached
        TOption_Normal     = osl_Process_NORMAL,     // run in normal window
        TOption_Hidden     = osl_Process_HIDDEN,     // run hidden
        TOption_Minimized  = osl_Process_MINIMIZED,  // run in minimized window
        TOption_Maximized  = osl_Process_MAXIMIZED,  // run in maximized window
        TOption_FullScreen = osl_Process_FULLSCREEN  // run in fullscreen window
    };

    /** Errorcodes:
    */
    enum TProcessError {
        E_None         = osl_Process_E_None,            /* no error */
        E_NotFound     = osl_Process_E_NotFound,        /* image not found */
        E_TimedOut     = osl_Process_E_TimedOut,        /* timout occured */
        E_NoPermission = osl_Process_E_NoPermission,    /* permission denied */
          E_Unknown    = osl_Process_E_Unknown,         /* unknown error */
        E_InvalidError = osl_Process_E_InvalidError     /* unmapped error */
    };

    enum TDescriptorFlags
    {
        TFlags_None = osl_Process_DFNONE,
        TFlags_Wait = osl_Process_DFWAIT
    };

    enum TProcessData
    {
        TData_Identifier = osl_Process_IDENTIFIER,
        TData_ExitCode   = osl_Process_EXITCODE,
        TData_CpuTimes   = osl_Process_CPUTIMES,
        TData_HeapUsage  = osl_Process_HEAPUSAGE
    };

    struct TProcessInfo : public oslProcessInfo
    {
        TProcessInfo() { Size = sizeof(*this); }
    };

    typedef oslProcessIdentifier TProcessIdentifier;

    /** Creating a process object by naming the executable.
        Does not yet start the process.
        @see execute
    */

    OProcess( );

    OProcess(const NAMESPACE_RTL(OUString)& strImageName);

      OProcess(const NAMESPACE_RTL(OUString)& strImageName,
             const NAMESPACE_RTL(OUString)& strWorkingDirectory);

    /// destroying a process object
    virtual ~OProcess();

    SAL_CALL operator oslProcess()
        { return m_Process; }

    SAL_CALL operator oslProcess() const
        { return m_Process; }

    static OProcess* SAL_CALL getProcess(TProcessIdentifier Identifier);

    /** execute the given process.
        This process becomes a child of the caller.
        If there are any ioresources provided from the calling process, this
        function returns only, if the child process calls OStartupInfo::acceptIOResource().
        @param Options [in] describes the execution mode.
        @return only not eNONE, if too much enviroments are added.
        @see OStartupInfo::acceptIOResource
    */
    TProcessError SAL_CALL execute(TProcessOption Options,
                          const OArgumentList& aArgumentList = OArgumentList(),
                          const OEnvironment&  aEnvironment  = OEnvironment()
                          );

    /** execute the given process with the specified security.
        This process becomes a child of the caller.
        The process is executed with the rights of the user, for whom the security object is created.
        If there are any ioresources provided from the calling process, this
        function returns only, if the child process calls OStartupInfo::acceptIOResource().
        @param Options [in] describes the execution mode.
        @param Security [in] is a given security object for one logged in user.
        @return eNONE, if the proccess could be executed, otherwise an errorcode.
        @see OStartupInfo::acceptIOResource
    */
    TProcessError SAL_CALL execute(TProcessOption Options,
                          const OSecurity &Security,
                          const OArgumentList& aArgumentList = OArgumentList(),
                          const OEnvironment&  aEnvironment  = OEnvironment()
                         );

    TProcessError SAL_CALL terminate();

    TProcessError SAL_CALL getInfo(TProcessData Data, TProcessInfo* pInfo) const;

    static TProcessError SAL_CALL getCurrentInfo(TProcessData Data, TProcessInfo* pInfo);

    /** wait for the completation of this child process
        @return eNONE if child process exits, otherwise nothing.
    */
    TProcessError SAL_CALL join();

    /** provide a socket as ioresource for the child process.
        The child has to call OStartUpInfo::acceptIOResources() if any
        ioresources are provided by calling this method, otherwise execute()
        will not return.
        This method should only be called before execute().
        @see OStartupInfo::acceptIOResource
    */
    void SAL_CALL provideIOResource(oslSocket Socket, TDescriptorFlags Flags = TFlags_Wait);

protected:
    const NAMESPACE_RTL(OUString) m_strImageName;
    const NAMESPACE_RTL(OUString) m_strDirectory;

    oslIOResource*     m_IoResources;
    sal_Int32          m_NoResources;
    oslProcess         m_Process;
};

/** informations for client processes provided by the parent.
    @see OProcess
*/


class OStartupInfo : public OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OStartupInfo, vos));

public:
    /** Errorcodes:
    */
    enum TStartupError {
        E_None         = osl_Process_E_None,            /* no error */
        E_NotFound     = osl_Process_E_NotFound,        /* image not found */
        E_TimedOut     = osl_Process_E_TimedOut,        /* timout occured */
        E_NoPermission = osl_Process_E_NoPermission,    /* permission denied */
          E_Unknown    = osl_Process_E_Unknown,         /* unknown error */
        E_InvalidError = osl_Process_E_InvalidError     /* unmapped error */
    };

    /** Constructor.
    */
    OStartupInfo();

    /** Destructor
    */
    ~OStartupInfo();

    /** get one ioresource from the parent process.
        The recieved socket has been opened by the parent process and will be
        duplicated for the calling process, wich has full read-write access to
        this socket.
        @param rSocket [out] returns the recieved socket.
        @return True, if the parent process provides resources for the caller, otherwise no return.
        @see OProcess::provideIOResource
    */
    sal_Bool SAL_CALL acceptIOResource(OSocket& rSocket);

    /** @return the number of command line arguments.
     */
    sal_uInt32 SAL_CALL getCommandArgCount();

    /** get the nArg-th command argument passed to the main-function of this process.
        @param nArg [in] the number of arguments to return.
        @param strCommandArg [out] the string that receives the argument.
        @return eNONE
    */
    TStartupError SAL_CALL getCommandArg(sal_uInt32 nArg, NAMESPACE_RTL(OUString)& strCommandArg);

    TStartupError SAL_CALL getExecutableFile(NAMESPACE_RTL(OUString)& strImageName);

    /** Get the value of one enviroment variable.
        @param Name [in] denotes the name of the variable to get.
        @param Buffer [out] is the buffer where the value of this variable is returned.
        @param Max [in] is the size of this buffer.
        @return eNONE, if the variable exist in the enviroment, otherwise False.
    */
    TStartupError SAL_CALL getEnvironment(const NAMESPACE_RTL(OUString)& strVar, NAMESPACE_RTL(OUString)& strValue);

protected:
    oslIOResource* m_IoResources;
    sal_Int32              m_NoResources;
};

#ifdef _USE_NAMESPACE
}
#endif

#endif  // _VOS_PROCESS_HXX_

