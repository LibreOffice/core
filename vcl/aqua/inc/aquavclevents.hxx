#ifndef INCLUDED_AQUAVCLEVENTS_HXX
#define INCLUDED_AQUAVCLEVENTS_HXX

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

/* Definition of custom OpenOffice.org events.

   Avoid conflict with Apple defined event class and type
   definitions by using uppercase letters. Lowercase
   letter definitions are reserved for Apple!
 */
enum {
  cOOoSalUserEventClass = 'OOUE'
};

enum {
  cOOoSalEventUser = 'UEVT',
  cOOoSalEventTimer = 'EVTT',
  cOOoSalEventData = 'EVTD',
  cOOoSalEventParamTypePtr = 'EPPT'
};

/* Definition of all necessary EventTypeSpec's */

const EventTypeSpec cWindowBoundsChangedEvent = { kEventClassWindow, kEventWindowBoundsChanged };
const EventTypeSpec cWindowCloseEvent = { kEventClassWindow, kEventWindowClose };
const EventTypeSpec cOOoSalUserEvent = { cOOoSalUserEventClass, cOOoSalEventUser };
const EventTypeSpec cOOoSalTimerEvent = { cOOoSalUserEventClass, cOOoSalEventTimer };
const EventTypeSpec cWindowActivatedEvent[] = { { kEventClassWindow, kEventWindowActivated },
                                                { kEventClassWindow, kEventWindowDeactivated } };
const EventTypeSpec cWindowPaintEvent = { kEventClassWindow, kEventWindowPaint };
const EventTypeSpec cWindowDrawContentEvent = { kEventClassWindow, kEventWindowDrawContent };

const EventTypeSpec cWindowFocusEvent[] = { { kEventClassWindow, kEventWindowFocusAcquired },
                                            { kEventClassWindow, kEventWindowFocusRelinquish } };

const EventTypeSpec cMouseEnterExitEvent[] = { { kEventClassControl, kEventControlTrackingAreaEntered },
                                               { kEventClassControl, kEventControlTrackingAreaExited } };

const EventTypeSpec cMouseEvent[] = { { kEventClassMouse, kEventMouseDown },
                                      { kEventClassMouse, kEventMouseUp },
                                      { kEventClassMouse, kEventMouseMoved },
                                      { kEventClassMouse, kEventMouseDragged } };
const EventTypeSpec cMouseWheelMovedEvent = { kEventClassMouse, kEventMouseWheelMoved };
const EventTypeSpec cWindowResizeStarted = { kEventClassWindow, kEventWindowResizeStarted };
const EventTypeSpec cWindowResizeCompleted = { kEventClassWindow, kEventWindowResizeCompleted };

/* Events for native menus */
const EventTypeSpec cCommandProcessEvent = { kEventClassCommand, kEventCommandProcess };
const EventTypeSpec cMenuPopulateEvent = { kEventClassMenu, kEventMenuPopulate };
const EventTypeSpec cMenuClosedEvent = { kEventClassMenu, kEventMenuClosed };
const EventTypeSpec cMenuTargetItemEvent = { kEventClassMenu, kEventMenuTargetItem };

/* Events for keyboard */
const EventTypeSpec cKeyboardRawKeyEvents[] = { { kEventClassKeyboard, kEventRawKeyDown},
                        { kEventClassKeyboard, kEventRawKeyUp},
                        { kEventClassKeyboard, kEventRawKeyRepeat},
                        { kEventClassKeyboard, kEventRawKeyModifiersChanged} };

const EventTypeSpec cTextInputEvents[] = { { kEventClassTextInput, kEventTextInputUpdateActiveInputArea},
                                           { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent},
                                           { kEventClassTextInput, kEventTextInputOffsetToPos} };

/* Events for scrollbar */

const EventTypeSpec cAppearanceScrollbarVariantChangedEvent = { kEventClassAppearance, kEventAppearanceScrollBarVariantChanged };

#endif // INCLUDED_AQUAVCLEVENTS_HXX
