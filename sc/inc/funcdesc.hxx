/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: funcdesc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:08:51 $
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

#ifndef SC_FUNCDESC_HXX
#define SC_FUNCDESC_HXX

/* Function descriptions for function wizard / autopilot / most recent used
 * list et al. Separated from the global.hxx lump, implementation still in
 * global.cxx
 */

#include <tools/list.hxx>
#include <tools/string.hxx>

#define MAX_FUNCCAT 12  /* maximum number of categories for functions */

class ScFuncDesc
{
public:

    struct ParameterFlags
    {
        bool    bOptional   :1;     // Parameter is optional
        bool    bSuppress   :1;     // Suppress parameter in UI because not implemented yet

        ParameterFlags() : bOptional(false), bSuppress(false) {}
    };

                ScFuncDesc();
                ~ScFuncDesc();

    void        Clear();
    void        InitArgumentInfo() const;

    /** Returns a semicolon separated list of all parameter names. */
    String  GetParamList        () const;
    /** Returns the full function signature: "FUNCTIONNAME( parameter list )". */
    String  GetSignature        () const;
    /** Returns the function signature with parameters from the passed string array. */
    String  GetFormulaString    ( String** aArgArr ) const;

    /** Returns the number of non-suppressed arguments. In case there are
        variable arguments the number of fixed non-suppressed arguments plus
        VAR_ARGS, same as for nArgCount (variable arguments can't be
        suppressed). */
    USHORT  GetSuppressedArgCount() const;

    /** Returns mapping from visible arguments to real arguments, e.g. if of 4
        parameters the second one is suppressed {0,2,3}. For VAR_ARGS
        parameters only one element is added to the end of the sequence. */
    ::std::vector<USHORT> GetVisibleArgMapping() const;

    String          *pFuncName;              // Function name
    String          *pFuncDesc;              // Description of function
    String         **ppDefArgNames;          // Parameter name(s)
    String         **ppDefArgDescs;          // Description(s) of parameter(s)
    ParameterFlags  *pDefArgFlags;           // Flags for each parameter
    USHORT           nFIndex;                // Unique function index
    USHORT           nCategory;              // Function category
    USHORT           nArgCount;              // All parameter count, suppressed and unsuppressed
    USHORT           nHelpId;                // HelpID of function
    bool             bIncomplete         :1; // Incomplete argument info (set for add-in info from configuration)
    bool             bHasSuppressedArgs  :1; // Whether there is any suppressed parameter.
};

//============================================================================

class ScFunctionList
{
public:
    ScFunctionList();
    ~ScFunctionList();

    ULONG           GetCount() const
                    { return aFunctionList.Count(); }

    const ScFuncDesc*   First()
                        { return (const ScFuncDesc*) aFunctionList.First(); }

    const ScFuncDesc*   Next()
                        { return (const ScFuncDesc*) aFunctionList.Next(); }

    const ScFuncDesc*   GetFunction( ULONG nIndex ) const
                    { return (const ScFuncDesc*) aFunctionList.GetObject( nIndex ); }

    xub_StrLen      GetMaxFuncNameLen() const
                    { return nMaxFuncNameLen; }

private:
    List        aFunctionList;
    xub_StrLen  nMaxFuncNameLen;
};

//============================================================================

class ScFunctionMgr
{
public:
                ScFunctionMgr();
                ~ScFunctionMgr();

    const ScFuncDesc*   Get( const String& rFName );
    const ScFuncDesc*   Get( USHORT nFIndex );
    const ScFuncDesc*   First( USHORT nCategory = 0 );
    const ScFuncDesc*   Next() const;

private:
    ScFunctionList* pFuncList;
    List*           aCatLists[MAX_FUNCCAT];
    List*           pCurCatList;
};

//==================================================================

class ScFormulaUtil
{
public:
    static BOOL                 GetNextFunc( const String&  rFormula,
                                             BOOL           bBack,
                                             xub_StrLen&    rFStart, // Ein- und Ausgabe
                                             xub_StrLen*    pFEnd = NULL,
                                             const ScFuncDesc** ppFDesc = NULL,
                                             String***      pppArgs = NULL );

    static const ScFuncDesc*    GetDefaultFuncDesc();

    static xub_StrLen           GetFunctionStart( const String& rFormula, xub_StrLen nStart,
                                                    BOOL bBack, String* pFuncName = NULL );

    static xub_StrLen           GetFunctionEnd  ( const String& rFormula, xub_StrLen nStart );

    static xub_StrLen           GetArgStart     ( const String& rFormula, xub_StrLen nStart,
                                                  USHORT nArg );

    static String**             GetArgStrings   ( const String& rFormula,
                                                  xub_StrLen    nFuncPos,
                                                  USHORT        nArgs );

    static void                 FillArgStrings  ( const String& rFormula,
                                                  xub_StrLen    nFuncPos,
                                                  USHORT        nArgs,
                                                  String**      pArgs );
};

//============================================================================

#endif // SC_FUNCDESC_HXX
