// -*- Mode: swift; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import UIKit
import QuickLookThumbnailing
import UniformTypeIdentifiers

// An app extension gets too little memory to run the engine, so the thumbnail is the icon of the
// application that opens the document.
class ThumbnailProvider: QLThumbnailProvider {
    struct UnsupportedFileError: Error {}

    // The cases are the types listed under QLSupportedContentTypes in Info.plist.
    static func iconName(forContentType contentType: UTType) -> String? {
        switch contentType.identifier {
        case "org.oasis-open.opendocument.text", "com.collabora.office.uti.fodt":
            return "x-office-document"
        case "org.oasis-open.opendocument.spreadsheet":
            return "x-office-spreadsheet"
        case "org.oasis-open.opendocument.presentation":
            return "x-office-presentation"
        case "com.collabora.office.uti.vsd":
            return "x-office-drawing"
        default:
            return nil
        }
    }

    override func provideThumbnail(for request: QLFileThumbnailRequest,
                                   _ handler: @escaping (QLThumbnailReply?, Error?) -> Void) {
        let fileURL = request.fileURL
        let contentType = (try? fileURL.resourceValues(forKeys: [.contentTypeKey]))?.contentType
            ?? UTType(filenameExtension: fileURL.pathExtension)
        guard let contentType,
              let iconName = Self.iconName(forContentType: contentType),
              let icon = UIImage(named: iconName) else {
            handler(nil, UnsupportedFileError())
            return
        }

        // A white portrait page with the icon in the middle.
        let maximum = request.maximumSize
        let minimum = request.minimumSize
        let pageRatio = sqrt(2.0)
        var height = min(maximum.height, maximum.width * pageRatio)
        var width = height / pageRatio
        height = min(max(height, minimum.height), maximum.height)
        width = min(max(width, minimum.width), maximum.width)
        let size = CGSize(width: width, height: height)
        let iconSide = min(size.width, size.height) * 0.6
        let iconRect = CGRect(x: (size.width - iconSide) / 2, y: (size.height - iconSide) / 2,
                              width: iconSide, height: iconSide)
        let reply = QLThumbnailReply(contextSize: size) {
            UIColor.white.setFill()
            UIRectFill(CGRect(origin: .zero, size: size))
            icon.draw(in: iconRect)
            return true
        }
        handler(reply, nil)
    }
}
