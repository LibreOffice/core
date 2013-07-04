//
//  slideShowPreviewTable_vc.m
//  iosremote
//
//  Created by Liu Siqi on 7/4/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowPreviewTable_vc.h"

@interface slideShowPreviewTable_vc ()

@property (nonatomic, strong) UIButton * startButton;

@end

@implementation slideShowPreviewTable_vc

@synthesize startButton = _startButton;

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
#warning Potentially incomplete method implementation.
    // Return the number of sections.
    return 0;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
#warning Incomplete method implementation.
    // Return the number of rows in the section.
    return 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
    
    // Configure the cell...
    
    return cell;
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

-(IBAction)startPresentationAction:(id)sender {
    
    UIAlertView *alert=[[UIAlertView alloc]initWithTitle:@"Reset"
                                                 message:@"You just pressed the Reset button"
                                                delegate:self
                                       cancelButtonTitle:@"Acknowledged"
                                       otherButtonTitles:nil];
    [alert show];
}

- (UIButton *)startButton{
    if (self.startButton == nil)
    {
        
        self.startButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
        self.startButton.frame = CGRectMake(20.0, 40 , 95.0, 37.0);
        [self.startButton setTitle:@"Start Presentation" forState:UIControlStateNormal];
        self.startButton.backgroundColor = [UIColor clearColor];
        [self.startButton addTarget:self action:@selector(startPresentationAction:) forControlEvents:UIControlEventTouchDown];
        
        self.startButton.tag = 1;    
    }
    return self.startButton;
}

- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section
{
    
    UIView* customView = [[UIView alloc] initWithFrame:CGRectMake(0.0, 0.0, 320.0, 100.0)];
    
    [customView addSubview:self.startButton];
    
    return customView;
}

- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
{
    return 100.0;
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Navigation logic may go here. Create and push another view controller.
    /*
     <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
     // ...
     // Pass the selected object to the new view controller.
     [self.navigationController pushViewController:detailViewController animated:YES];
     */
}

- (void)viewDidUnload {
    [super viewDidUnload];
}
@end
