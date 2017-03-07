//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit


private class FileStorage
{
    // house keeping variables
    private let filemgr          : FileManager = FileManager.default
    private var storageIsLocal   : Bool = true

    // Start path for the 2 storage locations
    private let baseLocalDocPath : URL
    private let baseCloudDocPath : URL?
    private var currrentDocPath  : URL? {
        get {
            return storageIsLocal ? baseLocalDocPath : baseCloudDocPath
        }
    }
    
    // make access to current dir independent of storage selection
    private var localDir  : URL
    private var cloudDir  : URL
    private var currentDir : URL {
        get {
            return storageIsLocal ? localDir : cloudDir
        }
        set(newDir) {
            if storageIsLocal {
               localDir = newDir
            } else {
               cloudDir = newDir
            }
        }
    }
    

    
    // content of current directory
    var currentFileList : [String] = []
    var currentDirList  : [String] = []
    
    
    
    // Support functions
    func iCloudEnabled() -> Bool
    {
        return baseCloudDocPath != nil
    }
    
    
    
    func selectStorage(_ doSwitch : Bool) -> Bool
    {
        if doSwitch {
            storageIsLocal = !storageIsLocal
        }
        return storageIsLocal
    }
    
    
    
    func enterDirectory(_ name: String)
    {
        // simple add directory
        currentDir = currentDir.appendingPathComponent(name)
        filemgr.changeCurrentDirectoryPath(name)
        buildFileList()
    }
    
    
    func leaveDirectory()
    {
        // step up for active storage, and only if not in root
        if currentDir != currrentDocPath {
            currentDir = currentDir.deletingLastPathComponent()
            buildFileList()
        }
    }
    
    
    
    func createDirectory(_ name: String)
    {
        let newDir = currentDir.appendingPathComponent(name)
        try! filemgr.createDirectory(at: newDir, withIntermediateDirectories: true, attributes: nil)
        enterDirectory(name + "/")
    }
    
    
    
    func deleteFileDirectory(_ name: String)
    {
        let delDir = currentDir.appendingPathComponent(name)
        try! filemgr.removeItem(at: delDir)
        buildFileList()
    }
    
    
    
    func getFileURL(_ name: String) -> URL
    {
        return currentDir.appendingPathComponent(name)
    }
    
    
    
    func copyFile(_ name: String)
    {
        try! filemgr.copyItem(at: currentDir.appendingPathComponent(name),
                              to: (storageIsLocal ? localDir : cloudDir).appendingPathComponent(name))
    }
    
    
    
    func moveFile(_ name: String)
    {
        try! filemgr.moveItem(at: currentDir.appendingPathComponent(name),
                              to: (storageIsLocal ? localDir : cloudDir).appendingPathComponent(name))
    }
    
    
    
    private func buildFileList()
    {
        currentDirList = []
        currentFileList = []
        let rawFileList = try! filemgr.contentsOfDirectory(at: currentDir,
                                                           includingPropertiesForKeys: [URLResourceKey.isDirectoryKey])
        for rawFile in rawFileList {
            var isDir: ObjCBool = false
            filemgr.fileExists(atPath: rawFile.path, isDirectory: &isDir)
            if isDir.boolValue {
                currentDirList.append(rawFile.lastPathComponent)
            } else {
                currentFileList.append(rawFile.lastPathComponent)
            }
        }
    }

    
    
    init()
    {
        baseLocalDocPath = filemgr.urls(for: .documentDirectory, in: .userDomainMask)[0]
        baseCloudDocPath = nil
        localDir         = baseLocalDocPath
        cloudDir         = baseLocalDocPath
        buildFileList()
    }
}



class FileManagerController : UITableViewController

{
    private var fileData = FileStorage()
    
    
    func showFiles()
    {
        
    }
    
    
    
    @IBAction func doSelectStorage(_ sender: UIBarButtonItem)
    {
        if fileData.selectStorage(true) {
            sender.title = "iCloud"
        }
        else {
            sender.title = "iPad"
        }
        showFiles()
    }
    
    
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        
        if segue.identifier == "ShowAttractionDetails" {
            
//            let detailViewController = segue.destination
//                as! AttractionDetailViewController
            
//            let myIndexPath = self.tableView.indexPathForSelectedRow!
//            let row = myIndexPath.row
//            detailViewController.webSite = webAddresses[row]
        }
    }
    
    
    
    override func numberOfSections(in tableView: UITableView) -> Int
    {
        return 1
    }
    
    
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int
    {
        return fileData.currentDirList.count + fileData.currentFileList.count
    }
    
    
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell
    {
        let cell = self.tableView.dequeueReusableCell(withIdentifier: "fileEntry", for: indexPath) as! FileManagerCell
        let row  = indexPath.row
        
        if row < fileData.currentDirList.count {
            cell.fileName = fileData.currentDirList[row]
            cell.fileLabel.text = cell.fileName + "/"
            cell.isDirectory  = true
        } else {
            let inx = row - fileData.currentDirList.count
            cell.fileName = fileData.currentFileList[inx]
            cell.fileLabel.text = cell.fileName
            cell.isDirectory  = false
        }
        return cell
    }


    // MARK: - ViewController basic
    override func viewDidLoad()
    {
        super.viewDidLoad()
        showFiles()
    }
}

class FileManagerCell: UITableViewCell {
        
    @IBOutlet weak var fileLabel: UILabel!
    var isDirectory : Bool = false
    var fileName    : String = ""
}



class FileManagerActions : UITableViewController
    
{
    @IBOutlet weak var buttonUploadDownload: UIButton!
    @IBOutlet weak var buttonDelete: UIButton!
    @IBOutlet weak var buttonOpen: UIButton!
    @IBOutlet weak var buttonLevelUp: UIButton!
    @IBOutlet weak var buttonCreateDirectory: UIButton!

    @IBAction func doOpen(_ sender: UIButton) {
        dismiss(animated: false)
    }
    @IBAction func doDelete(_ sender: UIButton) {
        dismiss(animated: false)
    }
    @IBAction func doUploadDownload(_ sender: UIButton) {
        dismiss(animated: false)
    }

    @IBAction func doLevelUp(_ sender: UIButton) {
        dismiss(animated: false)
    }
    @IBAction func doCreateDirectory(_ sender: UIButton) {
    }
    // MARK: - ViewController basic



    override func viewDidLoad()
    {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
    }
}
