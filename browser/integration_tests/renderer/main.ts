/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// The tests that draw what the engine writes run the browser code, which is not a set of modules.
// They build into one script with the sources and the canvas stand-ins of mocha_tests, the same
// way mocha_tests/main.ts builds.

// The tests build for an older language level than the browser code. Add the WeakRef and
// FinalizationRegistry types, which Node provides at run time.
/// <reference lib="es2021.weakref" />

// Load the interfaces first.
/// <reference path="../../src/control/jsdialog/Definitions.Types.ts" />
/// <reference path="../../src/app/iface/Clipboard.Interface.ts" />
/// <reference path="../../src/app/iface/Control.Interface.ts" />
/// <reference path="../../src/app/iface/DocLayer.Interface.ts" />
/// <reference path="../../src/app/iface/Map.Interface.ts" />
/// <reference path="../../src/app/iface/MapZoom.Interface.ts" />
/// <reference path="../../src/app/iface/SocketTypes.Interface.ts" />
/// <reference path="../../src/app/iface/TraceEvents.Interface.ts" />
/// <reference path="../../src/app/iface/Welcome.Interface.ts" />
/// <reference path="../../src/global.d.ts" />

/// <reference path="../../mocha_tests/refs/globals.ts" />
/// <reference path="../../mocha_tests/sources.ts" />
/// <reference path="../../mocha_tests/helper/CanvasRecorder.ts"/>
/// <reference path="../../mocha_tests/helper/ScratchCanvasStub.ts"/>
/// <reference path="../../mocha_tests/helper/Path2DRecorder.ts"/>
/// <reference path="../../mocha_tests/helper/ImageRecorder.ts"/>
/// <reference path="./VectorPrimitiveSamples.ts"/>

/// <reference path="./VectorPrimitiveRenderer.test.ts" />
