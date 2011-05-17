/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
 ************************************************************************/

#ifndef _MK_CREATE_HXX
#define _MK_CREATE_HXX

#include <tools/string.hxx>
//#include "bootstrp/sstring.hxx"

#include "bootstrp/prj.hxx"

class SvStream;
class SourceDirectoryList;

//
// class SourceDirectoryDependency
//

class CodedDependency : public ByteString
{
private:
    sal_uInt16 nOSType;                         // operating systems where dependeny exists

public:
    /* create a dependency instance with given coded directory name
     */
    CodedDependency(
        const ByteString &rCodedIdentifier, // the coded name of the directory
        sal_uInt16 nOperatingSystems            // the operating systems where this dependency exists
    ) :
    ByteString( rCodedIdentifier ),
    nOSType( nOperatingSystems )
    {
    }

    /* returns the operating system
     */
    sal_uInt16 GetOperatingSystem()
    {
        return nOSType;
    }

    /* set operating system
     */
    void SetOperatingSystem( sal_uInt16 nOperatingSystems )
    {
        nOSType = nOperatingSystems;
    }

    /* add operating systems if same dependency
     */
    sal_Bool TryToMerge(
        const ByteString &rCodedIdentifier, // the coded name of the directory
        sal_uInt16 nOperatingSystems            // the operating systems where this dependency exists
    )
    {
        if ( rCodedIdentifier != *this )
            return sal_False;
        nOSType |= nOperatingSystems;
        return sal_True;
    }
};

//
// class Dependecy
//

class Dependency : public ByteString
{
private:
    sal_uInt16 nOSType;                         // operating systems where dependecy exists

public:
    /* create a dependency instance with given directory name
     */
    Dependency(
        const ByteString &rDirectoryName,   // the coded name of the directory
        sal_uInt16 nOperatingSystems            // the operating systems where this dependency exists
    ) :
    ByteString( rDirectoryName ),
    nOSType( nOperatingSystems )
    {
    }

    /* returns the operating system
     */
    sal_uInt16 GetOperatingSystem()
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
    sal_uInt16 nOSType;                         // operating systems where this directory is used
    sal_uInt16 nDepth;                          // depth of directory structure (root is 0)

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
    static sal_uInt16 GetOSType(
        const ByteString &sDependExt        // the corresponding dependency extension (see also prj.hxx)
    );

    /* removes this and all sub directories with all dependencies
     */
    sal_Bool RemoveDirectoryTreeAndAllDependencies();

public:

    /* create a directory instance with given parent and name, no parent means this is the root
     * (not the file system root but the root of the source tree, e.g. o:\569)
     */
    SourceDirectory(
        const ByteString &rDirectoryName,           // name without parent
        sal_uInt16 nOperatingSystem,                    // the operating systems where this directory is used
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
    sal_uInt16 GetOperatingSystems() { return nOSType; }

    /* returns the given directory
     */
    SourceDirectory *GetDirectory(
        const ByteString &rDirectoryName,   // full path
        sal_uInt16 nOperatingSystem             // the operating systems where this directory is used
    );

    /* create the directory and all mandatory parents
     */
    SourceDirectory *InsertFull(
        const ByteString &rDirectoryName,   // full path
        sal_uInt16 nOperatingSystem             // the operating systems where this directory is used
    )
    {
        return GetDirectory( rDirectoryName, nOperatingSystem );
    }

    /* create the directory as sub directory of this directory
     */
    SourceDirectory *Insert(
        const ByteString &rDirectoryName,   // name without parent
        sal_uInt16 nOperatingSystem             // the operating systems where this directory is used
    );

    /* get the root directory
     */
    SourceDirectory *GetRootDirectory();

    /* get sub directory if exists
     */
    SourceDirectory *GetSubDirectory(
        const ByteString &rDirectoryPath,   // full sub path
        sal_uInt16 nOperatingSystem             // the operating systems where this directory is used
    );

    /* add a dependency for several platforms
     */
    CodedDependency *AddCodedDependency(
        const ByteString &rCodedIdentifier, // the coded name of the directory
        sal_uInt16 nOperatingSystems            // the operating systems where this dependency exists
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
        sal_uInt16 nOperatingSystems            // the operating systems where this dependency exists
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
        sal_Bool bAll = sal_False           // add all directories or only buildable ones
    );

    /* create the makefile.rc in file system
     */
    sal_Bool CreateRecursiveMakefile(
        sal_Bool bAllChilds = sal_False     // create rcursive for all sub directories
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
    size_t InsertSorted(
        SourceDirectory *pDirectory     // directory
    )
    {
        return PutString(( ByteString * ) pDirectory );
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
