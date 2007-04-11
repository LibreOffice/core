/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mkcreate.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:07:00 $
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

#ifndef _MK_CREATE_HXX
#define _MK_CREATE_HXX

#include <tools/string.hxx>

#ifndef _SSTRING_HXX
#include "bootstrp/sstring.hxx"
#endif

#include <tools/list.hxx>

#ifndef _PRJ_HXX
#include "bootstrp/prj.hxx"
#endif

class SvStream;
class SourceDirectoryList;

//
// class SourceDirectoryDependency
//

class CodedDependency : public ByteString
{
private:
    USHORT nOSType;                         // operating systems where dependeny exists

public:
    /* create a dependency instance with given coded directory name
     */
    CodedDependency(
        const ByteString &rCodedIdentifier, // the coded name of the directory
        USHORT nOperatingSystems            // the operating systems where this dependency exists
    ) :
    ByteString( rCodedIdentifier ),
    nOSType( nOperatingSystems )
    {
    }

    /* returns the operating system
     */
    USHORT GetOperatingSystem()
    {
        return nOSType;
    }

    /* set operating system
     */
    void SetOperatingSystem( USHORT nOperatingSystems )
    {
        nOSType = nOperatingSystems;
    }

    /* add operating systems if same dependency
     */
    BOOL TryToMerge(
        const ByteString &rCodedIdentifier, // the coded name of the directory
        USHORT nOperatingSystems            // the operating systems where this dependency exists
    )
    {
        if ( rCodedIdentifier != *this )
            return FALSE;
        nOSType |= nOperatingSystems;
        return TRUE;
    }
};

//
// class Dependecy
//

class Dependency : public ByteString
{
private:
    USHORT nOSType;                         // operating systems where dependecy exists

public:
    /* create a dependency instance with given directory name
     */
    Dependency(
        const ByteString &rDirectoryName,   // the coded name of the directory
        USHORT nOperatingSystems            // the operating systems where this dependency exists
    ) :
    ByteString( rDirectoryName ),
    nOSType( nOperatingSystems )
    {
    }

    /* returns the operating system
     */
    USHORT GetOperatingSystem()
    {
        return nOSType;
    }
};

//
// class SourceDirectory
//

class SourceDirectory : public ByteString
{
private:
    SourceDirectory *pParent;               // the parent directory
    SourceDirectoryList *pSubDirectories;   // list of sub directories
    USHORT nOSType;                         // operating systems where this directory is used
    USHORT nDepth;                          // depth of directory structure (root is 0)

    SByteStringList *pDependencies;         // dependencies on other directories in this depth

    SByteStringList *pCodedDependencies;    // dependencies on other directories in different depth
    SByteStringList *pCodedIdentifier;      // symbolic identifier to resolve dependencies

    /* try to resolve a single dependency
     */
    Dependency *ResolvesDependency(
        CodedDependency *pCodedDependency   // the dependency
    );

    /* returns the operating systems of a coded dependency
     */
    static USHORT GetOSType(
        const ByteString &sDependExt        // the corresponding dependency extension (see also prj.hxx)
    );

    /* removes this and all sub directories with all dependencies
     */
    BOOL RemoveDirectoryTreeAndAllDependencies();

public:

    /* create a directory instance with given parent and name, no parent means this is the root
     * (not the file system root but the root of the source tree, e.g. o:\569)
     */
    SourceDirectory(
        const ByteString &rDirectoryName,           // name without parent
        USHORT nOperatingSystem,                    // the operating systems where this directory is used
        SourceDirectory *pParentDirectory = NULL    // parent (if not root)
    );
    ~SourceDirectory();

    /* returns the full absolute path of this directory
     */
    ByteString GetFullPath();

    /* returns a list of all sub directories
     */
    SourceDirectoryList *GetSubDirectories() { return pSubDirectories; }

    /* returns the Operating systems where this directory is used
     */
    USHORT GetOperatingSystems() { return nOSType; }

    /* returns the given directory
     */
    SourceDirectory *GetDirectory(
        const ByteString &rDirectoryName,   // full path
        USHORT nOperatingSystem             // the operating systems where this directory is used
    );

    /* create the directory and all mandatory parents
     */
    SourceDirectory *InsertFull(
        const ByteString &rDirectoryName,   // full path
        USHORT nOperatingSystem             // the operating systems where this directory is used
    )
    {
        return GetDirectory( rDirectoryName, nOperatingSystem );
    }

    /* create the directory as sub directory of this directory
     */
    SourceDirectory *Insert(
        const ByteString &rDirectoryName,   // name without parent
        USHORT nOperatingSystem             // the operating systems where this directory is used
    );

    /* get the root directory
     */
    SourceDirectory *GetRootDirectory();

    /* get sub directory if exists
     */
    SourceDirectory *GetSubDirectory(
        const ByteString &rDirectoryPath,   // full sub path
        USHORT nOperatingSystem             // the operating systems where this directory is used
    );

    /* add a dependency for several platforms
     */
    CodedDependency *AddCodedDependency(
        const ByteString &rCodedIdentifier, // the coded name of the directory
        USHORT nOperatingSystems            // the operating systems where this dependency exists
    );

    /* returns the dependency list
     */
    SByteStringList *GetCodedDependencies()
    {
        return pCodedDependencies;
    }

    /* add symbolic identifier to resolve dependencies (to this directory and all parents)
     */
    CodedDependency *AddCodedIdentifier(
        const ByteString &rCodedIdentifier, // the coded name of the directory
        USHORT nOperatingSystems            // the operating systems where this dependency exists
    );

    /* returns the identifier list
     */
    SByteStringList *GetCodedIdentifier()
    {
        return pCodedIdentifier;
    }

    /* create dependencies on other directory, coded dependecies are used
     */
    void ResolveDependencies();

    /* returns the target definition for this directory (if dependencies exist)
     */
    ByteString GetTarget();

    /* returns the target definition for all sub directory
     */
    ByteString GetSubDirsTarget();

    /* create the full directory tree (only virtual, not in file system)
     */
    static SourceDirectory *CreateRootDirectory(
        const ByteString &rRoot,    // the root directory in file system
        const ByteString &rVersion, // the solar verion (r.g. SRC590, SRC591 etc.)
        BOOL bAll = FALSE           // add all directories or only buildable ones
    );

    /* create the makefile.rc in file system
     */
    BOOL CreateRecursiveMakefile(
        BOOL bAllChilds = FALSE     // create rcursive for all sub directories
    );
};

//
// class SourceDirectoryList
//

class SourceDirectoryList : public SByteStringList
{
public:
    /* create a empty directory list
     */
    SourceDirectoryList()
    {
    }
    ~SourceDirectoryList();

    /* search for a directory by directory name
     */
    SourceDirectory *Search(
        const ByteString &rDirectoryName    // name without parent
    );

    /* insert a new directory
     */
    ULONG InsertSorted(
        SourceDirectory *pDirectory     // directory
    )
    {
        return PutString(( ByteString * ) pDirectory );
    }
};

#endif
