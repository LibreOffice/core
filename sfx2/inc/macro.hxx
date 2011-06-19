/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _SFXMACRO_HXX
#define _SFXMACRO_HXX

//====================================================================
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <tools/string.hxx>

class SfxSlot;
class SfxShell;
struct SfxMacro_Impl;
class SfxMacro;

//====================================================================

class SfxMacroStatement
{
    sal_uInt16              nSlotId;    // performed Slot-Id or 0, if manually
    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > aArgs;      // current Parameter, in case nSlotId != 0
    String              aStatement; // Statement in BASIC-Syntax (if necessary with CR/LF)
    sal_Bool                bDone;      // comment out if no Done() is called
    void*               pDummy;     // for all cases, to remain compatible

#ifdef _SFXMACRO_HXX
private:
    void                GenerateNameAndArgs_Impl( SfxMacro *pMacro,
                                                  const SfxSlot &rSlot,
                                                  sal_Bool bRequestDone,
                                                  ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aArgs );
#endif

public:
                        SfxMacroStatement( const SfxMacroStatement &rOrig );

                        SfxMacroStatement( const String &rTarget,
                                           const SfxSlot &rSlot,
                                           sal_Bool bRequestDone,
                                           ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aArgs );

                        SfxMacroStatement( const SfxShell &rShell,
                                           const String &rTarget,
                                           sal_Bool bAbsolute,
                                           const SfxSlot &rSlot,
                                           sal_Bool bRequestDone,
                                           ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aArgs );

                        SfxMacroStatement( const String &rStatment );
                        ~SfxMacroStatement();

    sal_uInt16              GetSlotId() const;
    const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& GetArgs() const;
    sal_Bool                IsDone() const;
    const String&       GetStatement() const;
};

//--------------------------------------------------------------------

inline sal_uInt16 SfxMacroStatement::GetSlotId() const

/*  [Description]

    Returns the Slot-Id which the Statement should redo when called or 0
    if the Statement (<SFX_SLOT_RECORDMANUAL>) was recorded manually.
*/

{
    return nSlotId;
}

//--------------------------------------------------------------------

inline const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& SfxMacroStatement::GetArgs() const

/*  [Description]

    Returns the Parameter which was used in the call of the Statement or 0
    if the Statement (<SFX_SLOT_RECORDMANUAL>) was recorded manually

    The returned value belongs to the SfxMacroStatement and is only valid
    to the Stackframe.
*/

{
    return aArgs;
}

//--------------------------------------------------------------------

inline sal_Bool SfxMacroStatement::IsDone() const

/*  [Description]

    Returns TRUE only if the Statement really was excecuted, so for instance
    not if it was canceled by the user. If it was not excecuted the
    BASIC-Source would be commented out.
*/

{
    return bDone;
}

//--------------------------------------------------------------------

/*  [Description]

    Returns the Statement in BASIC-Syntax. If the Macro was created manually,
    the string will be returned exactly as given in the constructor, else the
    generated Source-Code is returned.

    It is possible in both of the above cases that several BASIC-Statements,
    each contains CR / LF separators in the string, since a SFx statement
    is translated if necessary into several BASIC statements.

    Statements for which <SfxRequest::Done()> was not called are marked
    with a preceding 'rem'.
*/

inline const String& SfxMacroStatement::GetStatement() const
{
    return aStatement;
}

//====================================================================

enum SfxMacroMode

/*  [Description]

    By using this enum it is determined, if a <SfxMacro>-Instance was created
    by absolute or relative recording, or to create a reference to a existing
    macro.
*/

{
    SFX_MACRO_EXISTING,           /* Indicates that it is a existing
                                     macro, which will only referenced */

    SFX_MACRO_RECORDINGABSOLUTE,  /* This macro shall be recorded so that the
                                     affected objects are to be addressed as
                                     directly as possible (example: "[doc.sdc]") */

    SFX_MACRO_RECORDINGRELATIVE   /* This macro is recorded and the affected
                                     objects are addressed in such a way so
                                     that when running the macro it refers to
                                     a vaild selection
                                     (example:" active document () ") */
};

//====================================================================

class SfxMacro

/*  [Description]

    Through using this class (and in fact its subclasses) the location of a
    BASIC function will be discribed (also in which Library, in which module
    and the Function name) as well as temporary storing a macro during the
    recording.
*/

{
    SfxMacro_Impl*          pImp;

public:
                            SfxMacro( SfxMacroMode eMode = SFX_MACRO_RECORDINGRELATIVE );
    virtual                 ~SfxMacro();

    SfxMacroMode            GetMode() const;
    void                    Record( SfxMacroStatement *pStatement );
    void                    Replace( SfxMacroStatement *pStatement );
    void                    Remove();
    const SfxMacroStatement*GetLastStatement() const;

    String                  GenerateSource() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
