/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

#include <QString>
#include <QWidget>

class QFileDialog;

struct SaveAsFormat
{
    QString action;      // e.g., "saveas-odt"
    QString extension;   // e.g., "odt"
    QString displayName; // e.g., "ODF text document (.odt)"
};

std::vector<SaveAsFormat> getSaveAsFormats(int docType);

// Shows the printer-selection dialog for an already-exported PDF file and
// prints or copies it as the user chooses. Removes the file's private temp
// directory when the dialog finishes.
void showPrintDialog(const std::string& tempFile, QWidget* parent);

void printDocument(unsigned appDocId, QWidget* parent = nullptr);

// Puts a file at its destination: a rename when the destination is on the
// same filesystem, otherwise an atomic copy (write to a temporary name
// beside the destination, then rename over it). An existing destination file
// stays intact when the operation fails. Returns true on success.
bool moveOrCopyFile(const std::string& fromPath, const std::string& toPath);

// Removes the private temporary directory holding the given exported file,
// with everything in it. Refuses, with an error log, a directory that does
// not lie inside the system temp directory, so a mangled path cannot lead to
// removing the user's own files.
void removeExportTempDirectory(const std::string& filePath);

// Asks the user where to save an already-written file and puts it there.
// Opens a save-file dialog over parent with the given title and suggested
// filename; picking a destination moves or atomically copies the file at
// srcPath to it, then runs onFinished with the outcome. A dismissed dialog
// never calls onFinished. The dialog deletes itself on close; the returned
// pointer allows connecting further cleanup to its signals.
QFileDialog* showSaveFileDialog(QWidget* parent, const QString& title,
                                const QString& suggestedName, const std::string& srcPath,
                                std::function<void(bool ok)> onFinished);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
