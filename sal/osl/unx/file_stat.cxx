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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include "osl/file.h"

#include "system.h"
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include "file_impl.hxx"
#include "file_error_transl.h"
#include "file_path_helper.hxx"
#include "file_url.h"
#include "uunxapi.hxx"

namespace /* private */
{

    inline void set_file_type(const struct stat& file_stat, oslFileStatus* pStat)
    {
        /* links to directories state also to be a directory */
       if (S_ISLNK(file_stat.st_mode))
           pStat->eType = osl_File_Type_Link;
       else if (S_ISDIR(file_stat.st_mode))
           pStat->eType = osl_File_Type_Directory;
       else if (S_ISREG(file_stat.st_mode))
           pStat->eType = osl_File_Type_Regular;
       else if (S_ISFIFO(file_stat.st_mode))
           pStat->eType = osl_File_Type_Fifo;
       else if (S_ISSOCK(file_stat.st_mode))
           pStat->eType = osl_File_Type_Socket;
       else if (S_ISCHR(file_stat.st_mode) || S_ISBLK(file_stat.st_mode))
           pStat->eType = osl_File_Type_Special;
       else
           pStat->eType = osl_File_Type_Unknown;

       pStat->uValidFields |= osl_FileStatus_Mask_Type;
    }

    inline void set_file_access_mask(const struct stat& file_stat, oslFileStatus* pStat)
    {
        // user permissions
        if (S_IRUSR & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_OwnRead;

        if (S_IWUSR & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_OwnWrite;

        if (S_IXUSR & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_OwnExe;

        // group permissions
        if (S_IRGRP & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_GrpRead;

        if (S_IWGRP & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_GrpWrite;

        if (S_IXGRP & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_GrpExe;

        // others permissions
        if (S_IROTH & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_OthRead;

        if (S_IWOTH & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_OthWrite;

        if (S_IXOTH & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_OthExe;

        pStat->uValidFields |= osl_FileStatus_Mask_Attributes;
    }

    inline void set_file_access_rights(const struct stat& file_stat, int S_IR, int S_IW, int S_IX, oslFileStatus* pStat)
    {
        /* we cannot really map osl_File_Attribute_ReadOnly to
           the Unix access rights, it's a Windows only flag
           that's why the following hack. We set osl_FileStatus_Mask_Attributes
           but if there is no read access for a file we clear the flag
           again to signal to the caller that there are no file attributes
           to read because that's better than to give them incorrect one.
        */
        pStat->uValidFields |= osl_FileStatus_Mask_Attributes;

        if ((0 == (S_IW & file_stat.st_mode)) && (S_IR & file_stat.st_mode))
            pStat->uAttributes |= osl_File_Attribute_ReadOnly;

        if (S_IX & file_stat.st_mode)
            pStat->uAttributes |= osl_File_Attribute_Executable;
    }

    /* a process may belong to up to NGROUPS_MAX groups, so when
       checking group access rights, we have to check all belonging
       groups */
    inline bool is_in_process_grouplist(const gid_t file_group)
    {
        // check primary process group

        if (getgid() == file_group)
            return true;

        // check supplementary process groups

        gid_t grplist[NGROUPS_MAX];
        int   grp_number = getgroups(NGROUPS_MAX, grplist);

        for (int i = 0; i < grp_number; i++)
        {
            if (grplist[i] == file_group)
                return true;
        }
        return false;
    }

    /* Currently we are determining the file access right based
       on the real user ID not the effective user ID!
       We don't use access(...) because access follows links which
       may cause performance problems see #97133.
    */
    inline void set_file_access_rights(const struct stat& file_stat, oslFileStatus* pStat)
    {
        if (getuid() == file_stat.st_uid)
        {
            set_file_access_rights(file_stat, S_IRUSR, S_IWUSR, S_IXUSR, pStat);
        }
        else if (is_in_process_grouplist(file_stat.st_gid))
        {
            set_file_access_rights(file_stat, S_IRGRP, S_IWGRP, S_IXGRP, pStat);
        }
        else
        {
            set_file_access_rights(file_stat, S_IROTH, S_IWOTH, S_IXOTH, pStat);
        }
    }

    inline void set_file_hidden_status(const rtl::OUString& file_path, oslFileStatus* pStat)
    {
        pStat->uAttributes   = osl::systemPathIsHiddenFileOrDirectoryEntry(file_path) ? osl_File_Attribute_Hidden : 0;
        pStat->uValidFields |= osl_FileStatus_Mask_Attributes;
    }

    /* the set_file_access_rights must be called after set_file_hidden_status(...) and
       set_file_access_mask(...) because of the hack in set_file_access_rights(...) */
    inline void set_file_attributes(
        const rtl::OUString& file_path, const struct stat& file_stat, const sal_uInt32 uFieldMask, oslFileStatus* pStat)
    {
        set_file_hidden_status(file_path, pStat);
        set_file_access_mask(file_stat, pStat);

        // we set the file access rights only on demand
        // because it's potentially expensive
        if (uFieldMask & osl_FileStatus_Mask_Attributes)
               set_file_access_rights(file_stat, pStat);
    }

    inline void set_file_access_time(const struct stat& file_stat, oslFileStatus* pStat)
    {
        pStat->aAccessTime.Seconds  = file_stat.st_atime;
        pStat->aAccessTime.Nanosec  = 0;
           pStat->uValidFields        |= osl_FileStatus_Mask_AccessTime;
    }

    inline void set_file_modify_time(const struct stat& file_stat, oslFileStatus* pStat)
    {
        pStat->aModifyTime.Seconds  = file_stat.st_mtime;
        pStat->aModifyTime.Nanosec  = 0;
        pStat->uValidFields        |= osl_FileStatus_Mask_ModifyTime;
    }

    inline void set_file_size(const struct stat& file_stat, oslFileStatus* pStat)
    {
        if (S_ISREG(file_stat.st_mode))
           {
            pStat->uFileSize     = file_stat.st_size;
               pStat->uValidFields |= osl_FileStatus_Mask_FileSize;
           }
    }

    /* we only need to call stat or lstat if one of the
       following flags is set */
    inline bool is_stat_call_necessary(sal_uInt32 field_mask, oslFileType file_type = osl_File_Type_Unknown)
    {
        return (
                ((field_mask & osl_FileStatus_Mask_Type) && (file_type == osl_File_Type_Unknown)) ||
                (field_mask & osl_FileStatus_Mask_Attributes) ||
                (field_mask & osl_FileStatus_Mask_CreationTime) ||
                (field_mask & osl_FileStatus_Mask_AccessTime) ||
                (field_mask & osl_FileStatus_Mask_ModifyTime) ||
                (field_mask & osl_FileStatus_Mask_FileSize) ||
                (field_mask & osl_FileStatus_Mask_LinkTargetURL) ||
                (field_mask & osl_FileStatus_Mask_Validate));
    }

    inline oslFileError set_link_target_url(const rtl::OUString& file_path, oslFileStatus* pStat)
    {
        rtl::OUString link_target;
        if (!osl::realpath(file_path, link_target))
            return oslTranslateFileError(OSL_FET_ERROR, errno);

        oslFileError osl_error = osl_getFileURLFromSystemPath(link_target.pData, &pStat->ustrLinkTargetURL);
        if (osl_error != osl_File_E_None)
            return osl_error;

        pStat->uValidFields |= osl_FileStatus_Mask_LinkTargetURL;
        return osl_File_E_None;
    }

    inline oslFileError setup_osl_getFileStatus(
        DirectoryItem_Impl * pImpl, oslFileStatus* pStat, rtl::OUString& file_path)
    {
        if ((NULL == pImpl) || (NULL == pStat))
            return osl_File_E_INVAL;

        file_path = rtl::OUString(pImpl->m_ustrFilePath);
        OSL_ASSERT(file_path.getLength() > 0);
        if (file_path.getLength() <= 0)
            return osl_File_E_INVAL;

        pStat->uValidFields = 0;
        return osl_File_E_None;
    }

} // end namespace private


/****************************************************************************
 *  osl_getFileStatus
 ****************************************************************************/

oslFileError SAL_CALL osl_getFileStatus(oslDirectoryItem Item, oslFileStatus* pStat, sal_uInt32 uFieldMask)
{
    DirectoryItem_Impl * pImpl = static_cast< DirectoryItem_Impl* >(Item);

    rtl::OUString file_path;
    oslFileError  osl_error = setup_osl_getFileStatus(pImpl, pStat, file_path);
    if (osl_File_E_None != osl_error)
        return osl_error;

#if defined(__GNUC__) && (__GNUC__ < 3)
    struct ::stat file_stat;
#else
    struct stat file_stat;
#endif

    bool bStatNeeded = is_stat_call_necessary(uFieldMask, pImpl->getFileType());
    if (bStatNeeded && (0 != osl::lstat(file_path, file_stat)))
        return oslTranslateFileError(OSL_FET_ERROR, errno);

    if (bStatNeeded)
    {
        // we set all these attributes because it's cheap
        set_file_type(file_stat, pStat);
        set_file_access_time(file_stat, pStat);
        set_file_modify_time(file_stat, pStat);
        set_file_size(file_stat, pStat);
        set_file_attributes(file_path, file_stat, uFieldMask, pStat);

        // file exists semantic of osl_FileStatus_Mask_Validate
        if ((uFieldMask & osl_FileStatus_Mask_LinkTargetURL) && S_ISLNK(file_stat.st_mode))
        {
            osl_error = set_link_target_url(file_path, pStat);
            if (osl_error != osl_File_E_None)
                return osl_error;
        }
    }
#ifdef _DIRENT_HAVE_D_TYPE
    else if (uFieldMask & osl_FileStatus_Mask_Type)
    {
        pStat->eType = pImpl->getFileType();
        pStat->uValidFields |= osl_FileStatus_Mask_Type;
    }
#endif /* _DIRENT_HAVE_D_TYPE */

    if (uFieldMask & osl_FileStatus_Mask_FileURL)
    {
        if ((osl_error = osl_getFileURLFromSystemPath(file_path.pData, &pStat->ustrFileURL)) != osl_File_E_None)
            return osl_error;

        pStat->uValidFields |= osl_FileStatus_Mask_FileURL;
    }

    if (uFieldMask & osl_FileStatus_Mask_FileName)
    {
        osl_systemPathGetFileNameOrLastDirectoryPart(file_path.pData, &pStat->ustrFileName);
        pStat->uValidFields |= osl_FileStatus_Mask_FileName;
       }
    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_setFileAttributes */
/****************************************************************************/

static oslFileError osl_psz_setFileAttributes( const sal_Char* pszFilePath, sal_uInt64 uAttributes )
{
    oslFileError osl_error = osl_File_E_None;
    mode_t       nNewMode  = 0;

     OSL_ENSURE(!(osl_File_Attribute_Hidden & uAttributes), "osl_File_Attribute_Hidden doesn't work under Unix");

    if (uAttributes & osl_File_Attribute_OwnRead)
        nNewMode |= S_IRUSR;

    if (uAttributes & osl_File_Attribute_OwnWrite)
        nNewMode|=S_IWUSR;

    if  (uAttributes & osl_File_Attribute_OwnExe)
        nNewMode|=S_IXUSR;

    if (uAttributes & osl_File_Attribute_GrpRead)
        nNewMode|=S_IRGRP;

    if (uAttributes & osl_File_Attribute_GrpWrite)
        nNewMode|=S_IWGRP;

    if (uAttributes & osl_File_Attribute_GrpExe)
        nNewMode|=S_IXGRP;

    if (uAttributes & osl_File_Attribute_OthRead)
        nNewMode|=S_IROTH;

    if (uAttributes & osl_File_Attribute_OthWrite)
        nNewMode|=S_IWOTH;

    if (uAttributes & osl_File_Attribute_OthExe)
        nNewMode|=S_IXOTH;

    if (chmod(pszFilePath, nNewMode) < 0)
        osl_error = oslTranslateFileError(OSL_FET_ERROR, errno);

    return osl_error;
}

oslFileError SAL_CALL osl_setFileAttributes( rtl_uString* ustrFileURL, sal_uInt64 uAttributes )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return osl_psz_setFileAttributes( path, uAttributes );
}

/****************************************************************************/
/*  osl_setFileTime */
/****************************************************************************/

static oslFileError osl_psz_setFileTime (
    const sal_Char* pszFilePath,
    const TimeValue* /*pCreationTime*/,
    const TimeValue* pLastAccessTime,
    const TimeValue* pLastWriteTime )
{
    int nRet=0;
    struct utimbuf aTimeBuffer;
    struct stat aFileStat;
#ifdef DEBUG_OSL_FILE
    struct tm* pTM=0;
#endif

    nRet = lstat(pszFilePath,&aFileStat);

    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"File Times are (in localtime):\n");
    pTM=localtime(&aFileStat.st_ctime);
    fprintf(stderr,"CreationTime is '%s'\n",asctime(pTM));
    pTM=localtime(&aFileStat.st_atime);
    fprintf(stderr,"AccessTime   is '%s'\n",asctime(pTM));
    pTM=localtime(&aFileStat.st_mtime);
    fprintf(stderr,"Modification is '%s'\n",asctime(pTM));

    fprintf(stderr,"File Times are (in UTC):\n");
    fprintf(stderr,"CreationTime is '%s'\n",ctime(&aFileStat.st_ctime));
    fprintf(stderr,"AccessTime   is '%s'\n",ctime(&aTimeBuffer.actime));
    fprintf(stderr,"Modification is '%s'\n",ctime(&aTimeBuffer.modtime));
#endif

    if ( pLastAccessTime != 0 )
    {
        aTimeBuffer.actime=pLastAccessTime->Seconds;
    }
    else
    {
        aTimeBuffer.actime=aFileStat.st_atime;
    }

    if ( pLastWriteTime != 0 )
    {
        aTimeBuffer.modtime=pLastWriteTime->Seconds;
    }
    else
    {
        aTimeBuffer.modtime=aFileStat.st_mtime;
    }

    /* mfe: Creation time not used here! */

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"File Times are (in localtime):\n");
    pTM=localtime(&aFileStat.st_ctime);
    fprintf(stderr,"CreationTime now '%s'\n",asctime(pTM));
    pTM=localtime(&aTimeBuffer.actime);
    fprintf(stderr,"AccessTime   now '%s'\n",asctime(pTM));
    pTM=localtime(&aTimeBuffer.modtime);
    fprintf(stderr,"Modification now '%s'\n",asctime(pTM));

    fprintf(stderr,"File Times are (in UTC):\n");
    fprintf(stderr,"CreationTime now '%s'\n",ctime(&aFileStat.st_ctime));
    fprintf(stderr,"AccessTime   now '%s'\n",ctime(&aTimeBuffer.actime));
    fprintf(stderr,"Modification now '%s'\n",ctime(&aTimeBuffer.modtime));
#endif

    nRet=utime(pszFilePath,&aTimeBuffer);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    return osl_File_E_None;
}

oslFileError SAL_CALL osl_setFileTime (
    rtl_uString* ustrFileURL,
    const TimeValue* pCreationTime,
    const TimeValue* pLastAccessTime,
    const TimeValue* pLastWriteTime )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return osl_psz_setFileTime( path, pCreationTime, pLastAccessTime, pLastWriteTime );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
