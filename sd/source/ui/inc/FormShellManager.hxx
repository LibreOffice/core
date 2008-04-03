/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormShellManager.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:54:31 $
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

#ifndef SD_FORM_SHELL_MANAGER_HXX
#define SD_FORM_SHELL_MANAGER_HXX

#include <ViewShellManager.hxx>

#include <tools/link.hxx>
#include <svtools/lstner.hxx>

class VclWindowEvent;
class FmFormShell;

namespace sd { namespace tools { class EventMultiplexerEvent; } }

namespace sd {

class PaneManagerEvent;
class ViewShell;
class ViewShellBase;

/** This simple class is responsible for putting the form shell above or
    below the main view shell on the shell stack maintained by the ObjectBarManager.

    The form shell is moved above the view shell when the form shell is
    activated, i.e. the FormControlActivated handler is called.

    It is moved below the view shell when the main window of the
    main view shell is focused.

    The form shell is created and destroyed by the ViewShellManager by using
    a factory object provided by the FormShellManager.
*/
class FormShellManager
    : public SfxListener
{
public:
    FormShellManager (ViewShellBase& rBase);
    virtual ~FormShellManager (void);

    /** Typically called by a ShellFactory.  It tells the
        FormShellManager which form shell to manage.
        @param pFormShell
            This may be <NULL/> to disconnect the ViewShellManager from the
            form shell.
    */
    void SetFormShell (FmFormShell* pFormShell);

    /** Return the form shell last set with SetFormShell().
        @return
            The result may be <NULL/> when the SetFormShell() method has not
            yet been called or was last called with <NULL/>.
    */
    FmFormShell* GetFormShell (void);

private:
    ViewShellBase& mrBase;

    /** Ownership of the form shell lies with the ViewShellManager.  This
        reference is kept so that the FormShellManager can detect when a new
        form shell is passed to SetFormShell().
    */
    FmFormShell* mpFormShell;

    /** Remember whether the form shell is currently above or below the main
        view shell.
    */
    bool mbFormShellAboveViewShell;

    /** The factory is remembered so that it removed from the
        ViewShellManager when the FormShellManager is destroyed.
    */
    ViewShellManager::SharedShellFactory mpSubShellFactory;

    bool mbIsMainViewChangePending;

    ::Window* mpMainViewShellWindow;

    /** Register at window of center pane and at the form shell that
        represents the form tool bar.  The former informs this manager about
        the deselection of the form shell.  The later informs about its
        selection.
    */
    void RegisterAtCenterPane (void);

    /** Unregister the listeners that were registered in
        RegisterAtCenterPane().
    */
    void UnregisterAtCenterPane (void);

    /** This call back is called by the application window (among others)
        when the window gets the focus.  In this case the form shell is
        moved to the bottom of the shell stack.
    */
    DECL_LINK(WindowEventHandler, VclWindowEvent*);

    /** This call back is called when view in the center pane is replaced.
        When this happens then we unregister at the window of the old and
        register at the window of the new shell.
    */
    DECL_LINK(ConfigurationUpdateHandler, ::sd::tools::EventMultiplexerEvent*);

    /** This call back is called by the form shell when it gets the focus.
        In this case the form shell is moved to the top of the shell stack.
    */
    DECL_LINK(FormControlActivated, FmFormShell*);

    /** This method is called by the form shell when that is destroyed.  It
        acts as a last resort against referencing a dead form shell.  With
        the factory working properly this method should not be necessary
        (and may be removed in the future.)
    */
    virtual void SFX_NOTIFY(
        SfxBroadcaster& rBC,
        const TypeId& rBCType,
        const SfxHint& rHint,
        const TypeId& rHintType);
};

} // end of namespace sd

#endif
