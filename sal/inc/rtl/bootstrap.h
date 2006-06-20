/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bootstrap.h,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:13:51 $
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
#ifndef _RTL_BOOTSTRAP_H_
#define _RTL_BOOTSTRAP_H_

#include <rtl/ustring.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   @HTML
   @file

   The described concept provides a platform independent way to access
   minimum bootstrap settings for every application by excplitly or
   implicitly passing the values to the application.<p>

   5-LEVEL STRATEGY FOR RETRIEVAL OF BOOTSTRAP VALUES :<p>

   The 1st level is tried first. On failure,
   the next level is tried. Every query starts at the first level again, so
   that one setting may be taken from the 3rd and one from the 1st level.<p>

   1st level: explicitly set variables via rtl_bootstrap_set()

   2nd level: command line arguments. A "-env:SETTINGNAME=value" is given on
   command line. This allows to give an application a certain setting, even
   if an ini-file exists (espicially useful for e.g. daemons that want to
   start an executable with dynamical changing settings).<p>

   3rd level: environment variables. The application tries to get the
   setting from the environment.<p>

   4th level: ini-files. Every application looks for an ini-file.
   The filename defaults to /absoulte/path/to/executable[rc|.ini]
   (without .bin or .exe suffix). The ini-filename can be
   set by the special command line parameter
   '-env:INIFILENAME=/absolute/path/to/inifile' at runtime or it may
   be set at compiletime by an API-call.<p>

   5th level: default. An application can have some default settings decided
   at compile time, which allow the application to run even with no
   deployment settings. <p>

   If neither of the 5 levels leads to an successful retrieval of the value
   (no default possible), the application may  fail to start.<p>

   NAMING CONVENTIONS <p>

   Naming conventions for names of bootstrap values :
   Names may only include characters, that are allowed charcters for
   environment variables. This excludes '.', ' ', ';', ':' and any non-ascii
   character. Names are case insensitive.<p>

   The ini-file is only allowed to have one section, which must be named '[Bootstrap]'.
   The section may be omitted.
   The section name does not appear in the name of the corresponding
   environment variable or commandline arg.
   Values maybe arbitrary unicode strings, they must be encoded in UTF8.<p>

   Example:<p>

   in an ini-file:
   <code>
   [Sectionname]
   Name=value
   </code><p>

   as commandline arg:
   <code>-env:Name=value</code><p>

   as environment
   <code>
   setenv Name value
   set Name=value
   </code><p>

   SPECIAL VARIABLES:

   <ul>
   <li> INIFILENAME<br>
     This variable allows to set the inifilename. This makes only sense, if the filename
     is different than the executable file name. It must be given on command line. If it is
     given the executable ini-file is ignored.
   </li>
*/

/** may be called by an application to set an ini-filename.

    <p>
    Must be called before rtl_bootstrap_get(). May not be called twice.
    If it is never called, a the filename executable.ini (win)
    or execuablerc (unx) is assumed.

    @param pName Name of the inifile with path but WITHOUT
    suffix (.ini or rc)
*/
void SAL_CALL rtl_bootstrap_setIniFileName( rtl_uString *pName )
    SAL_THROW_EXTERN_C();

/**
   @param ppValue
        out parameter. Contains always a valid rtl_uString pointer.
   @param pName
            The name of the bootstrap setting to be     retrieved.
   @param pDefault
        maybe NULL. If once the default is
           returned, successive calls always return this
           default value, even when called with different
           defaults.

   @return <code>sal_True</code>, when a value could be retrieved successfully,
           <code>sal_False</code>, when none of the 4 methods gave a value. ppValue
           then contains ane empty string.
           When a pDefault value is given, the function returns always
           <code>sal_True</code>.
*/
sal_Bool SAL_CALL rtl_bootstrap_get( rtl_uString *pName, rtl_uString **ppValue, rtl_uString *pDefault )
    SAL_THROW_EXTERN_C();

/** Sets a bootstrap parameter.

   @param pName
          name of bootstrap parameter
   @param pValue
          value of bootstrap parameter
*/
void SAL_CALL rtl_bootstrap_set( rtl_uString * pName, rtl_uString * pValue )
    SAL_THROW_EXTERN_C();


typedef void * rtlBootstrapHandle;

/**
   Opens a bootstrap argument container.
   @param pIniName [in]   The name of the ini-file to use, if <code>NULL</code> defaults
                          to the excutables name
   @return                Handle for a boostrap argument container
*/
rtlBootstrapHandle SAL_CALL rtl_bootstrap_args_open(rtl_uString * pIniName)
    SAL_THROW_EXTERN_C();

/**
   Closes a boostrap agument container.
   @param handle [in]     The handle got by <code>rtl_bootstrap_args_open()</code>
*/
void SAL_CALL rtl_bootstrap_args_close(rtlBootstrapHandle handle)
    SAL_THROW_EXTERN_C();

/**
   @param handle   [in]     The handle got by <code>rtl_bootstrap_args_open()</code>
   @param pName    [in]     The name of the variable to be retrieved
   @param ppValue  [out]    The result of the retrieval. *ppValue may be null in case of failure.
   @param pDefault [in]     The default value for the retrieval, may be <code>NULL</code>

   @return                  The status of the retrieval, <code>sal_True</code> on success.
*/
sal_Bool SAL_CALL rtl_bootstrap_get_from_handle(rtlBootstrapHandle handle, rtl_uString *pName, rtl_uString **ppValue, rtl_uString *pDefault)
    SAL_THROW_EXTERN_C();


/** Returns the name of the inifile associated with this handle.

   @param ppIniName contains after the call the name of the ini-filename.
*/
void SAL_CALL rtl_bootstrap_get_iniName_from_handle(rtlBootstrapHandle handle, rtl_uString ** ppIniName)
    SAL_THROW_EXTERN_C();

/** Expands a macro using bootstrap variables.

    @param handle   [in]     The handle got by <code>rtl_bootstrap_args_open()</code>
    @param macro    [inout]  The macro to be expanded
*/
void SAL_CALL rtl_bootstrap_expandMacros_from_handle(
    rtlBootstrapHandle handle, rtl_uString ** macro )
    SAL_THROW_EXTERN_C();
/** Expands a macro using default bootstrap variables.

    @param macro    [inout]  The macro to be expanded
*/
void SAL_CALL rtl_bootstrap_expandMacros(
    rtl_uString ** macro )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
