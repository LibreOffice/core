/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localfilehelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 15:11:34 $
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
#include "precompiled_configmgr.hxx"
#ifndef CONFIGMGR_LOCALBE_LOCALFILEHELPER_HXX_
#include "localfilehelper.hxx"
#endif

#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#include <vector>

namespace configmgr
{
 namespace localbe
 {
    //------------------------------------------------------------------------------
    bool isValidFileURL (rtl::OUString const& _sFileURL)
    {
        using osl::File;

        rtl::OUString sSystemPath;
        return _sFileURL.getLength() && (File::E_None == File::getSystemPathFromFileURL(_sFileURL, sSystemPath));
    }
    //------------------------------------------------------------------------------
    void validateFileURL(const rtl::OUString& _sFileURL, const uno::Reference<uno::XInterface>& pContext)
        throw(css::configuration::InvalidBootstrapFileException)
    {
        if (!isValidFileURL( _sFileURL))
        {
            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii(" Not a Valid File URL: \"");
            sMsg.append(_sFileURL);
            sMsg.appendAscii("\"");
            throw com::sun::star::configuration::InvalidBootstrapFileException(
                sMsg.makeStringAndClear(),pContext, rtl::OUString() ) ;
        }
    }
    //------------------------------------------------------------------------------
    void checkFileExists(const rtl::OUString& _sFileURL,const uno::Reference<uno::XInterface>& pContext)
        throw (backend::CannotConnectException)
    {
        if (!FileHelper::fileExists(_sFileURL))
        {
            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii(" No Such File or Directory: \"");
            sMsg.append(_sFileURL);
            sMsg.appendAscii("\"");

            throw backend::CannotConnectException(sMsg.makeStringAndClear(), pContext, uno::Any()) ;
        }
    }
    //------------------------------------------------------------------------------
    void checkIfDirectory(const rtl::OUString& _sFileURL, const uno::Reference<uno::XInterface>& pContext)
        throw (backend::BackendSetupException)
    {
        if (!FileHelper::dirExists(_sFileURL))
        {
            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii(" File:\"");
            sMsg.append(_sFileURL);
            sMsg.appendAscii("\" Must be a Directory\"");

            throw backend::BackendSetupException(sMsg.makeStringAndClear(),pContext, uno::Any()) ;
        }
    }

    // ---------------------------------------------------------------------------------------
    bool implEnsureAbsoluteURL(rtl::OUString & _rsURL) // also strips embedded dots etc.
    {
        using osl::File;

        rtl::OUString sBasePath = _rsURL;
        OSL_VERIFY(osl_Process_E_None == osl_getProcessWorkingDir(&sBasePath.pData));

        rtl::OUString sAbsolute;
        if ( File::E_None == File::getAbsoluteFileURL(sBasePath, _rsURL, sAbsolute))
        {
            _rsURL = sAbsolute;
            return true;
        }
        else
        {
            OSL_ENSURE(false, "Could not get absolute file URL for valid URL");
            return false;
        }
    }
    // ---------------------------------------------------------------------------------------
    osl::DirectoryItem::RC implNormalizeURL(rtl::OUString & _sURL, osl::DirectoryItem& aDirItem)
    {
        using namespace osl;

        OSL_PRECOND(aDirItem.is(), "Opened DirItem required");

        static const sal_uInt32 cFileStatusMask = FileStatusMask_FileURL;

        FileStatus aFileStatus(cFileStatusMask);

        DirectoryItem::RC rc = aDirItem.getFileStatus(aFileStatus);

        if (rc == DirectoryItem::E_None)
        {
            rtl::OUString aNormalizedURL = aFileStatus.getFileURL();

            if (aNormalizedURL.getLength() != 0)
                _sURL = aNormalizedURL;
            else
                rc = DirectoryItem::E_INVAL;
        }
        return rc;
    }

    // ---------------------------------------------------------------------------------------

    bool normalizeURL(rtl::OUString & _sURL,  const uno::Reference<uno::XInterface>& pContext, bool bNothrow )
      throw (backend::InsufficientAccessRightsException, backend::BackendAccessException)
    {

        using namespace osl;

        if (_sURL.getLength() == 0)
            return false;

        DirectoryItem aDirItem;

        DirectoryItem::RC rc = DirectoryItem::get(_sURL, aDirItem);

        if (rc == DirectoryItem::E_None)
            rc = implNormalizeURL(_sURL,aDirItem);

        switch (rc)
        {
        case DirectoryItem::E_None:  return true;

        case DirectoryItem::E_NOENT: return true;

        case DirectoryItem::E_ACCES:
           if (!bNothrow)
           {
               rtl::OUStringBuffer msg;
               msg.appendAscii("LocalBackend: Cannot normalize URL \"" );
               msg.append(_sURL);
               msg.appendAscii("\" - InsufficientAccess");
               throw backend::InsufficientAccessRightsException(msg.makeStringAndClear(),pContext,uno::Any());
           }
           return false;

        default:
            if (!bNothrow)
            {
                rtl::OUStringBuffer msg;
                msg.appendAscii("LocalBackend: Cannot normalize URL \"" );
                msg.append(_sURL);
                msg.appendAscii("\" - ").append(FileHelper::createOSLErrorString(rc));
                throw backend::BackendAccessException(msg.makeStringAndClear(),pContext,uno::Any());
            }
            return false;

        }
    }

    // ---------------------------------------------------------------------------------------
    static const sal_Unicode kComponentSeparator = '.' ;
    static const sal_Unicode kPathSeparator = '/' ;

    rtl::OUString componentToPath(const rtl::OUString& aComponent)
    {
        rtl::OUStringBuffer retCode ;

        retCode.append(kPathSeparator) ;
        retCode.append(aComponent.replace(kComponentSeparator, kPathSeparator)) ;
        return retCode.makeStringAndClear() ;
    }
    //------------------------------------------------------------------------------
    rtl::OUString layeridToPath(const rtl::OUString& aLayerId)
    {
        sal_Int32 const nSplit = aLayerId.indexOf(k_cLayerIdSeparator);
        if (nSplit < 0)
            return componentToPath(aLayerId);

        rtl::OUString const aComponent= aLayerId.copy(0,nSplit);
        rtl::OUString const aSubid = aLayerId.copy(nSplit+1);

        rtl::OUStringBuffer retCode ;

        retCode.append(kPathSeparator) ;
        retCode.append(aComponent.replace(kComponentSeparator, kPathSeparator)) ;
        retCode.append(kPathSeparator) ;
        retCode.append(aSubid) ;
        return retCode.makeStringAndClear() ;
    }
    //------------------------------------------------------------------------------
    bool checkOptionalArg(rtl::OUString& aArg)
    {
        if (aArg.getLength() && aArg[0] == sal_Unicode('?'))
        {
            aArg = aArg.copy(1);
            return true;
        }
        else
        {
            return false;
        }
    }
    //------------------------------------------------------------------------------
    void fillFromBlankSeparated(const rtl::OUString& aList,
                                uno::Sequence<rtl::OUString>& aTarget)
    {
        std::vector<rtl::OUString> tokens ;
        sal_Int32 nextToken = 0 ;

        do {
            tokens.push_back(aList.getToken(0, ' ', nextToken)) ;
        } while (nextToken >= 0) ;
        if (tokens.size() > 0) {
            aTarget.realloc(tokens.size()) ;
            std::vector<rtl::OUString>::const_iterator token ;
            sal_Int32 i = 0 ;

            for (token = tokens.begin() ; token != tokens.end() ; ++ token) {
                aTarget [i ++] = *token ;
            }
        }
    }
    //------------------------------------------------------------------------------
  }
} // namespace configmgr
