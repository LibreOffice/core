/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import UIKit
import QuickLook

class PreviewViewController: UIViewController, QLPreviewingController {
    override func viewDidLoad() {
        super.viewDidLoad()
    }

    func preparePreviewOfFile(at url: URL) async throws {
        let titleText = UILabel();
        titleText.textAlignment = .center;
        titleText.text = String(localized: "This file can't be previewed");
        titleText.font = UIFont(descriptor: titleText.font.fontDescriptor, size: 30);
        titleText.adjustsFontSizeToFitWidth = true;
        titleText.adjustsFontForContentSizeCategory = true;
        titleText.translatesAutoresizingMaskIntoConstraints = false;
        titleText.minimumScaleFactor = 0.5;
        titleText.numberOfLines = 0;
        view.addSubview(titleText);
        
        let appName = Bundle.main.infoDictionary?["COMainAppName"] as! String;
        
        let subtitleText = UILabel();
        subtitleText.textAlignment = .center;
        subtitleText.text = String(localized: "To read or edit this file, open it in \(appName)");
        subtitleText.font = UIFont(descriptor: subtitleText.font.fontDescriptor, size: -20);
        subtitleText.adjustsFontSizeToFitWidth = true;
        subtitleText.adjustsFontForContentSizeCategory = true;
        subtitleText.translatesAutoresizingMaskIntoConstraints = false;
        subtitleText.minimumScaleFactor = 0.5;
        subtitleText.numberOfLines = 0;
        view.addSubview(subtitleText);
        
        let icon = UIImageView()
        icon.image = UIImage(named: "AppIcon1024x1024");
        icon.translatesAutoresizingMaskIntoConstraints = false;
        view.addSubview(icon);
        
        NSLayoutConstraint.activate([
            titleText.centerXAnchor.constraint(equalTo: view.centerXAnchor),
            subtitleText.centerXAnchor.constraint(equalTo: view.centerXAnchor),
            icon.centerXAnchor.constraint(equalTo: view.centerXAnchor),
            
            titleText.leftAnchor.constraint(equalTo: view.leftAnchor, constant: 20),
            subtitleText.leftAnchor.constraint(equalTo: view.leftAnchor, constant: 20),
            
            subtitleText.topAnchor.constraint(equalTo: titleText.lastBaselineAnchor, constant: 10),
            icon.bottomAnchor.constraint(equalTo: titleText.topAnchor, constant: -10),
            titleText.centerYAnchor.constraint(equalTo: view.centerYAnchor),
            
            icon.widthAnchor.constraint(equalToConstant: 128),
            icon.heightAnchor.constraint(equalToConstant: 128),
        ]);
    }
}
