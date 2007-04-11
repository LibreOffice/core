/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: geninfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:11:53 $
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
#ifndef _BOOTSTRP_GENINFO_HXX
#define _BOOTSTRP_GENINFO_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#include <tools/string.hxx>
#include <tools/list.hxx>

// forwards
class GenericInformationList;

//
// class GenericInformation
//

/******************************************************************************
Purpose: holds generic informations and subinformations in a simple format
******************************************************************************/

class TOOLS_DLLPUBLIC GenericInformation : public ByteString    // the key is stored in base class
{
friend class GenericInformationList;    // can be child or/and parent
private:
    ByteString sValue;                  // holds value of data
    ByteString sComment;

    GenericInformationList *pInfoList;  // holds subinformations
    GenericInformationList *pParent;    // holds a pointer to parent list

    // methods
    void ListDeleted() { pParent = NULL; }  // allowed to be accessed
                                                // from friend class
                                                // GenericInformationList

public:
    GenericInformation( const ByteString &rKey, const ByteString &rValue,
                        GenericInformationList *pParentList = NULL,
                        GenericInformationList *pSubInfos = NULL );
    GenericInformation( const GenericInformation& rInf, BOOL bCopySubs = TRUE);

    ~GenericInformation();

  ByteString &GetValue() { return sValue; }
  void SetValue( const ByteString &rValue ) { sValue = rValue; }

  ByteString &GetComment() { return sComment; }
  void SetComment( const ByteString &rComment ) { sComment = rComment; }

    // this methods used to handle sub informations
    BOOL InsertSubInfo( GenericInformation *pInfo );
  // siehe GenericInformationList
  BOOL InsertSubInfo( const ByteString &rPathKey, const ByteString &rValue,
              BOOL bSearchByPath = FALSE, BOOL bNewPath = FALSE);
    void RemoveSubInfo( GenericInformation *pInfo, BOOL bDelete = FALSE );
  //  void RemoveSelf( BOOL bDelete = FALSE ); // loescht sich selbst aus der Parentliste
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // bei bDelete = TRUE werden auch alle Sublisten UND DIE INFO SELBST geloescht.
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    GenericInformation *GetSubInfo( ByteString &rKey, BOOL bSearchByPath = FALSE,
                    BOOL bCreatePath = FALSE );
                // path can be something like this: src370/drives/o:

    void SetSubList( GenericInformationList *pSubList )
        { pInfoList = pSubList; }
    GenericInformationList *GetSubList() { return pInfoList; }
};

/* diese Klasse bietet einen SemaphoreLock zum lesen (und nicht Schreiben)
 * oder schreiben (und nicht lesen)
 */
class GenericLockInformation : public GenericInformation
{
public:
GenericLockInformation( const ByteString &rKey, const ByteString &rValue,
            GenericInformationList *pParentList = NULL,
            GenericInformationList *pSubInfos = NULL )
  : GenericInformation( rKey, rValue, pParentList, pSubInfos),
    aLockState( read ), nLockKey( 0 ) {};
  //~GenericLockInformation();

  /* bietet einen Lockmechanismus fuer exclusive Zugriffe
   *
   * -"writeonly" wird angesprochen, wenn von der Wurzel ausgehend
   *  ein Item veraendert werden soll. In der Zeit kann die Liste nicht
   *  gelesen werden, womit keine Inconsistenzen entstehen koennen.
   *
   * -"read" wird zum Normalen lesen der Infos benutzt, 90% der Betriebszeit.
   *  waerenddessen kann nicht geschrieben werden -> writeonly Lock.
   *  Ist fuer den atomaren(nicht unterbrochenen) Lesezugriff gedacht
   *
   * -"readonly" wird zum niederschreiben des Teilbaums benutzt, was schon mal
   *  10 Minuten dauern kann. In der Zeit kann kein Writeonlylock gesetzt
   *  werden, aber ein rescedule. Damit koennen andere Aktionen asynchron ausgefuert
   *  werden, aber die Datensicherheit bleibt gewahrt
   *
   * Zustandsaenderung: writeonly <-> read <-> readonly
   *
   * nLockKey ist zum verschluesseln des LockZugriffs mit einem 32bit Wort vorgesehen.
   * ist der Schluessel nicht null, so kann nur mit dem Schluessel in
   * die Baumstruktur geschrieben werden.
   * ist der Schluessel nLockKey Null, dann kann jeder Schreiben und die Locks loesen
   */
  enum LockState{ writeonly, read, readonly };

  /* der Schreibschutz darf nur aktiviert werden, wenn
   * der Status auf Lesen steht
   */
  BOOL SetWriteLock(UINT32 nKey = 0) { return ((read==aLockState) &&
                           (aLockState=writeonly, nLockKey=nKey, TRUE)); }
  /* Schreibschutz darf nur geloest werden, wenn
   * der Schreibschutz drin ist, und
   * entweder der LockKey Null ist(Generalschluessel) oder der Key zum LockKey passt
   */
  BOOL ReleaseWriteLock(UINT32 nKey = 0) { return ((writeonly==aLockState) &&
                           (!nLockKey||nKey==nLockKey) &&
                           (aLockState=read, nLockKey=0, TRUE)); } // setzt den zustand auf "read"
  BOOL SetReadLock(UINT32 nKey = 0) { return ((read==aLockState) &&
                          (aLockState=readonly, nLockKey=nKey, TRUE)); }
  BOOL ReleaseReadLock(UINT32 nKey = 0) { return ((readonly==aLockState) &&
                          (!nLockKey||nKey==nLockKey) &&
                          (aLockState=read, nLockKey=0, TRUE)); } // setzt den zustand auf "read"

  LockState GetLockState() const { return aLockState; }
  BOOL IsWriteLocked() const { return (writeonly==aLockState); }
  BOOL IsReadLocked() const { return (readonly==aLockState); }
  BOOL IsNotLocked() const { return (read==aLockState); }
  BOOL IsLocker( UINT32 nKey ) { return (nKey==nLockKey || !nLockKey); }

  /* wenn der Schreibschutz aktiviert wurde,
   * und bei vorhandenem Schreibschutz die Keys stimmen
   * rufe die Parentmethode auf */
  BOOL InsertSubInfo( GenericInformation *pInfo, UINT32 nKey = 0 ) {
    return ((writeonly==aLockState) &&
        (!nLockKey || nKey==nLockKey) &&
        (GenericInformation::InsertSubInfo( pInfo ), TRUE)); }

  BOOL InsertSubInfo( const ByteString &rPathKey, const ByteString &rValue, UINT32 nKey = 0,
              BOOL bSearchByPath = FALSE, BOOL bNewPath = FALSE) {
    return ((writeonly==aLockState) &&
        (!nLockKey || nKey==nLockKey) &&
        (GenericInformation::InsertSubInfo( rPathKey, rValue, bSearchByPath, bNewPath ), TRUE)); }
  /* 29.jan.98: erweiterung um lesemoeglichkeit vom Lockclienten */
  GenericInformation *GetSubInfo( ByteString &rKey, BOOL bSearchByPath = FALSE,
                  BOOL bCreatePath = FALSE, UINT32 nKey = 0 ) {
    if (writeonly==aLockState && nLockKey && nKey!=nLockKey )
      return NULL;
    return GenericInformation::GetSubInfo(rKey, bSearchByPath, bCreatePath); }

  //  TYPEINFO();
private:

  LockState aLockState;
  UINT32    nLockKey;
};

//
// class GenericInformationList
//

/******************************************************************************
Purpose: holds set of generic informations in a sorted list
******************************************************************************/

DECLARE_LIST( GenericInformationList_Impl, GenericInformation * )

class TOOLS_DLLPUBLIC GenericInformationList : public GenericInformationList_Impl
{
private:
    GenericInformation *pOwner;         // holds parent of this list

protected:
    // methods
    ULONG InsertSorted( GenericInformation *pInfo, BOOL bOverwrite,
                            ULONG nStart, ULONG nEnd );
    GenericInformation *Search( ULONG &rPos, ByteString sKey,
                            ULONG nStart, ULONG nEnd );

public:
    GenericInformationList( GenericInformation *pParent = NULL );
    GenericInformationList(const GenericInformationList& rList, GenericInformation *pParent = NULL);
    ~GenericInformationList();

    // this methods used to handle the informations using binary search
    GenericInformation *GetInfo( ByteString &rKey, BOOL bSearchByPath = FALSE,
                     BOOL bCreatePath = FALSE );
  /* path can be something like this: src370/drives/o:
   * bCreatePath will create the neccecary paths to the GI */

    BOOL InsertInfo( GenericInformation *pInfo, BOOL bOverwrite = TRUE );
  /* legt eine GenericInformation im Baum an mit Key-Value
   * wenn bNewPath gesetzt, wird der nichtexistente Teil des Pfades neu kreiert
   * wenn bNewPath nicht gesetzt ist und ein Teil des Pfades nicht vorhanden ist,
   * gibt die Methode FALSE zurueck.*/
  BOOL InsertInfo( const ByteString &rPathKey, const ByteString &rValue,
           BOOL bSearchByPath = FALSE, BOOL bNewPath = FALSE);
    void RemoveInfo( GenericInformation *pInfo, BOOL bDelete = FALSE );

    GenericInformation* SetOwner( GenericInformation *pNewOwner );

};

#endif

