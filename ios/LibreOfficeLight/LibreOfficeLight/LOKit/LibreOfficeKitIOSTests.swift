//
//  LibreOfficeKitIOSTests.swift
//  LibreOfficeKitIOSTests
//
//  Created by Jon Nermut on 30/12/17.
//  Copyright © 2017 LibreOffice. All rights reserved.
//

import XCTest
@testable import LibreOfficeKitIOS

class LibreOfficeKitIOSTests: XCTestCase {

    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }

    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }



    func testLoadingSimpleDoc() {

        guard let lo = try? LibreOffice() else
        {
            XCTFail("Could not start LibreOffice")
            return
        }

        let b = Bundle.init(for: LibreOfficeKitIOSTests.self)
        guard let url = b.url(forResource: "test-page-format", withExtension: "docx") else
        {
            XCTFail("Failed to get url to test doc")
            return
        }

        var loCallbackCount = 0
        lo.registerCallback()
        {
            typ, payload in
            print(typ)
            print(payload)
            loCallbackCount += 1
        }

        guard let doc = try? lo.documentLoad(url: url.absoluteString) else
        {
            XCTFail("Could not load document")
            return
        }

        var docCallbackCount = 0
        doc.registerCallback()
        {
            typ, payload in
            print(typ)
            print(payload)
            docCallbackCount += 1
        }

        //let typ: LibreOfficeDocumentType = doc.getDocumentType()
        //XCTAssertTrue(typ == LibreOfficeDocumentType.LOK_DOCTYPE_TEXT)

        doc.initializeForRendering()
        let rects = doc.getPartRectangles()
        print(rects) // 284, 284, 12240, 15840; 284, 16408, 12240, 15840
        let tileMode = doc.getTileMode()
        print(tileMode) // 1
        let canvasSize = CGSize(width: 1024,height: 1024)
        let tile = CGRect(x: 284, y: 284, width: 12240, height: 12240)


        guard let image = doc.paintTileToImage(canvasSize: canvasSize, tileRect: tile) else
        {
            XCTFail("No image")
            return
        }
        if let data = UIImagePNGRepresentation(image)
        {
            let filename = getDocumentsDirectory().appendingPathComponent("tile1.png")
            try? data.write(to: filename)
            print("Wrote tile to: \(filename)")
        }
    }

}

func getDocumentsDirectory() -> URL
{
    let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
    return paths[0]
}

public extension Document
{

}

