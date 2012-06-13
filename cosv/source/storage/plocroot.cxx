/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#define CSV_USE_CSV_ASSERTIONS
#include <cosv/csv_env.hxx>

#include <cosv/comfunc.hxx>
#include <cosv/string.hxx>
#include <cosv/streamstr.hxx>
#include <cosv/std_outp.hxx>
#include <cosv/tpl/dyn.hxx>
#include <cosv/ploc.hxx>

// NOT FULLY DECLARED SERVICES
#include <ctype.h>
#include <cosv/bstream.hxx>
#include <cosv/csv_ostream.hxx>


namespace csv
{
namespace ploc
{


class UnixRootDir : public Root
{
  public:
                        UnixRootDir();

    virtual void        Get(
                            ostream      &      o_rPath ) const;
    virtual void        Get(
                            bostream      &     o_rPath ) const;
    virtual DYN Root *  CreateCopy() const;
    virtual const char *
                        OwnDelimiter() const;
};

class WorkingDir : public Root
{
  public:
                        WorkingDir(
                            const char *        i_sDelimiter = Delimiter() );

    virtual void        Get(
                            ostream      &      o_rPath ) const;
    virtual void        Get(
                            bostream      &     o_rPath ) const;
    virtual DYN Root *  CreateCopy() const;
    virtual const char *
                        OwnDelimiter() const;
  private:
    String              sOwnDelimiter;
};

class WinRootDir : public Root
{
  public:
                        WinRootDir();

    virtual void        Get(
                            ostream      &      o_rPath ) const;
    virtual void        Get(
                            bostream      &     o_rPath ) const;
    virtual DYN Root *  CreateCopy() const;
    virtual const char *
                        OwnDelimiter() const;
};

class WinDrive : public Root
{
  public:
                        WinDrive(
                            char                i_cDrive );
    virtual void        Get(
                            ostream      &      o_rPath ) const;
    virtual void        Get(
                            bostream      &     o_rPath ) const;
    virtual DYN Root *  CreateCopy() const;
    virtual const char *
                        OwnDelimiter() const;
  private:
    char                cDrive;
};

class WinDriveRootDir : public Root
{
  public:
                        WinDriveRootDir(
                            const char *        i_sPath );
                        WinDriveRootDir(
                            char                i_cDrive );

    virtual void        Get(
                            ostream      &      o_rPath ) const;
    virtual void        Get(
                            bostream      &     o_rPath ) const;
    virtual DYN Root *  CreateCopy() const;
    virtual const char *
                        OwnDelimiter() const;
  private:
    char                cDrive;
};

class UNCRoot : public Root
{
  public:
                        UNCRoot(
                            const char *        i_sPath );
                        UNCRoot(
                            const String  &     i_sComputer,
                            const String  &     i_sEntryPt );

    virtual void        Get(
                            ostream      &      o_rPath ) const;
    virtual void        Get(
                            bostream      &     o_rPath ) const;
    virtual DYN Root *  CreateCopy() const;
    virtual const char *
                        OwnDelimiter() const;
  private:
    String              sComputer;
    String              sEntryPt;
};

class InvalidRoot : public Root
{
  public:
    virtual void        Get(
                            ostream      &      o_rPath ) const;
    virtual void        Get(
                            bostream      &     o_rPath ) const;
    virtual DYN Root *  CreateCopy() const;
    virtual const char *
                        OwnDelimiter() const;
};


DYN Root *
Create_WindowsRoot( const char * &  o_sPathAfterRoot,
                    const char *    i_sPath )
{
    if (i_sPath[0] == '\\')
    {
         if (i_sPath[1] == '\\')
        {   // UNC path name
             o_sPathAfterRoot = strchr(i_sPath+2,'\\');
            if (o_sPathAfterRoot != 0)
            {
                o_sPathAfterRoot = strchr(o_sPathAfterRoot+1,'\\');
                if (o_sPathAfterRoot != 0)
                    ++o_sPathAfterRoot;
                return new UNCRoot(i_sPath);
            }
            return new InvalidRoot;   // Incomplete UNC root.
        }
        else
        {
            o_sPathAfterRoot = i_sPath+1;
             return new WinRootDir;
        }
    }
    else if (i_sPath[1] == ':')
    {
        if ( i_sPath[2] == '\\')
        {
            o_sPathAfterRoot = i_sPath + 3;
            return new WinDriveRootDir(i_sPath);
        }
        else
        {
            o_sPathAfterRoot = i_sPath + 2;
            return new WinDrive(*i_sPath);
        }
    }
    else
    {
        o_sPathAfterRoot = i_sPath;
        return new WorkingDir("\\");
    }
}

DYN Root *
Create_UnixRoot( const char * &     o_sPathAfterRoot,
                 const char *       i_sPath )
{
    if (*i_sPath == '/')
    {
        o_sPathAfterRoot = i_sPath + 1;
        return new UnixRootDir;
    }
    else //
    {
        o_sPathAfterRoot = i_sPath;
        return new WorkingDir("/");
    }  // endif
}


//**********************    Root    ****************************//

Root::~Root()
{

}

DYN Root *
Root::Create_( const char * &   o_sPathAfterRoot,
               const char *     i_sPath,
               const char *     i_sDelimiter )
{
    if (i_sPath[0] == '.')
    {
        switch ( i_sPath[1] )
        {
             case '\0':  o_sPathAfterRoot = i_sPath + 1;
                        break;
            case '\\':  o_sPathAfterRoot = i_sPath + 2;
                        break;
            case '/':   o_sPathAfterRoot = i_sPath + 2;
                        break;
            case '.':   o_sPathAfterRoot = i_sPath;
                        break;
            default:
                        o_sPathAfterRoot = 0;
                        return new InvalidRoot;
        }   // end switch (i_sPath[1])

        return new WorkingDir;
    }   // end if (i_sPath[0] == '.')

    switch (*i_sDelimiter)
    {
         case '\\':      return Create_WindowsRoot(o_sPathAfterRoot, i_sPath);
        case '/':       return Create_UnixRoot(o_sPathAfterRoot, i_sPath);
    }

    o_sPathAfterRoot = 0;
    return new InvalidRoot;
}



//**********************    UnixRootDir    ****************************//


UnixRootDir::UnixRootDir()
{
}

void
UnixRootDir::Get( ostream      &      o_rPath ) const
{
    o_rPath << '/';
}

void
UnixRootDir::Get( bostream      &     o_rPath ) const
{
    o_rPath.write( "/", 1 );
}

DYN Root *
UnixRootDir::CreateCopy() const
{
     return new UnixRootDir;
}

const char *
UnixRootDir::OwnDelimiter() const
{
     return "/";
}


//**********************    WorkingDir    ****************************//

WorkingDir::WorkingDir( const char * i_sDelimiter )
    :   sOwnDelimiter(i_sDelimiter)
{
}

void
WorkingDir::Get( ostream      &      o_rPath ) const
{
    o_rPath << '.' << sOwnDelimiter;
}

void
WorkingDir::Get( bostream      &     o_rPath ) const
{
    o_rPath.write( ".", 1 );
    o_rPath.write( sOwnDelimiter );
}

DYN Root *
WorkingDir::CreateCopy() const
{
     return new WorkingDir(sOwnDelimiter);
}

const char *
WorkingDir::OwnDelimiter() const
{
     return sOwnDelimiter;
}


//**********************    WinRootDir    ****************************//

WinRootDir::WinRootDir()
{
}

void
WinRootDir::Get( ostream      &      o_rPath ) const
{
    o_rPath << '\\';
}

void
WinRootDir::Get( bostream      &     o_rPath ) const
{
    o_rPath.write( "\\", 1 );
}

DYN Root *
WinRootDir::CreateCopy() const
{
     return new WinRootDir;
}

const char *
WinRootDir::OwnDelimiter() const
{
     return "\\";
}


//**********************    WinDrive    ****************************//

WinDrive::WinDrive( char i_cDrive )
    :   cDrive(static_cast< char >(toupper(i_cDrive)))
{
}

void
WinDrive::Get( ostream      &      o_rPath ) const
{
    o_rPath << cDrive << ':';
}

void
WinDrive::Get( bostream      &     o_rPath ) const
{
    static char buf_[3] = " :";
    buf_[0] = cDrive;
    o_rPath.write( &buf_[0], 2 );
}

DYN Root *
WinDrive::CreateCopy() const
{
     return new WinDrive(cDrive);
}

const char *
WinDrive::OwnDelimiter() const
{
     return "\\";
}


//**********************    WinDriveRootDir    ****************************//

WinDriveRootDir::WinDriveRootDir( const char * i_sPath )
    :   cDrive(static_cast< char >(toupper(*i_sPath)))
{
    if ( 'A' > cDrive OR 'Z' < cDrive )
        cDrive = 0;
}

WinDriveRootDir::WinDriveRootDir( char i_cDrive )
    :   cDrive(i_cDrive)
{
}

void
WinDriveRootDir::Get( ostream      &      o_rPath ) const
{
    o_rPath << cDrive << ":\\";
}

void
WinDriveRootDir::Get( bostream      &     o_rPath ) const
{
    static char buf_[4] = " :\\";
    buf_[0] = cDrive;
    o_rPath.write( &buf_[0], 3 );
}

DYN Root *
WinDriveRootDir::CreateCopy() const
{
     return new WinDriveRootDir(cDrive);
}

const char *
WinDriveRootDir::OwnDelimiter() const
{
     return "\\";
}


//**********************    UNCRoot    ****************************//

UNCRoot::UNCRoot( const char * i_sPath )
//  :   // sComputer,
        // sEntryPt
{
    const char * pRestPath = i_sPath + 2;
    const char * pDirEnd = strchr(pRestPath, '\\');
    csv_assert(pDirEnd != 0);

    sComputer = String(pRestPath, pDirEnd - pRestPath);
    pRestPath = pDirEnd+1;
    pDirEnd = strchr(pRestPath, '\\');

    if ( pDirEnd != 0 )
    {
        sEntryPt = String(pRestPath, pDirEnd - pRestPath);
    }
    else
    {
        sEntryPt = pRestPath;
    }
}

UNCRoot::UNCRoot( const String  &     i_sComputer,
                  const String  &     i_sEntryPt )
    :   sComputer(i_sComputer),
        sEntryPt(i_sEntryPt)
{
}

void
UNCRoot::Get( ostream      & o_rPath ) const
{
    o_rPath << "\\\\" << sComputer << '\\' << sEntryPt << "\\";
}

void
UNCRoot::Get( bostream      &     o_rPath ) const
{
    o_rPath.write( "\\\\", 2 );
    o_rPath.write( sComputer );
    o_rPath.write(  "\\", 1 );
    o_rPath.write( sEntryPt );
    o_rPath.write(  "\\", 1 );
}

DYN Root *
UNCRoot::CreateCopy() const
{
     return new UNCRoot(sComputer,sEntryPt);
}

const char *
UNCRoot::OwnDelimiter() const
{
     return "\\";
}



//**********************    InvalidRoot    ****************************//

void
InvalidRoot::Get( ostream      & ) const
{
}

void
InvalidRoot::Get( bostream      & ) const
{
}

DYN Root *
InvalidRoot::CreateCopy() const
{
     return new InvalidRoot;
}

const char *
InvalidRoot::OwnDelimiter() const
{
    return 0;
}




} // namespace ploc
} // namespace csv



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
