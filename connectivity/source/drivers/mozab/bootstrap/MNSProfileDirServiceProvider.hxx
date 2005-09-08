/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSProfileDirServiceProvider.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:25:16 $
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

// Interfaces Needed
#include "nsIDirectoryService.h"
#include "nsILocalFile.h"
#include "nsString.h"

// Forward Declarations
class nsIAtom;

// --------------------------------------------------------------------------------------
// nsProfileDirServiceProvider - The nsIDirectoryServiceProvider implementation used for
// profile-relative file locations.
// --------------------------------------------------------------------------------------

class nsProfileDirServiceProvider: public nsIDirectoryServiceProvider
{
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDIRECTORYSERVICEPROVIDER

  friend nsresult NS_NewProfileDirServiceProvider( nsProfileDirServiceProvider**);

public:

   virtual nsresult        SetProfileDir(nsIFile* aProfileDir);
   virtual nsresult         Register();
   virtual nsresult         Shutdown();

protected:
                           nsProfileDirServiceProvider();
   virtual                 ~nsProfileDirServiceProvider();

  nsresult                 Initialize();
  nsresult                 EnsureProfileFileExists(nsIFile* profileDir);
  nsresult                 UndefineFileLocations();

protected:

  // Atoms for file locations
  static nsIAtom*          sApp_PrefsDirectory50;
  static nsIAtom*          sApp_PreferencesFile50;
  static nsIAtom*          sApp_UserProfileDirectory50;

  nsCOMPtr<nsIFile>        mProfileDir;

};


// --------------------------------------------------------------------------------------

/**
 * Global method to create an instance of nsProfileDirServiceProvider
 *
 * @param aNotifyObservers    If true, will send out profile startup
 *                            notifications when the profile directory is set.
 *                            See nsIProfileChangeStatus.
 */

nsresult NS_NewProfileDirServiceProvider( nsProfileDirServiceProvider** aProvider);

