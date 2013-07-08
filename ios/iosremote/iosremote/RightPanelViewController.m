//
//  RightPanelViewController.m
//  SlideoutNavigation
//
//  Created by Tammy Coron on 1/10/13.
//  Copyright (c) 2013 Tammy L Coron. All rights reserved.
//

#import "RightPanelViewController.h"

#import "Animal.h"

@interface RightPanelViewController ()

@property (nonatomic, weak) IBOutlet UITableView *myTableView;
@property (nonatomic, weak) IBOutlet UITableViewCell *cellMain;

@property (nonatomic, strong) NSMutableArray *arrayOfAnimals;

@end

@implementation RightPanelViewController

#pragma mark -
#pragma mark View Did Load/Unload

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self setupAnimalsArray];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
}

#pragma mark -
#pragma mark View Will/Did Appear

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
	[super viewDidAppear:animated];
}

#pragma mark -
#pragma mark View Will/Did Disappear

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

#pragma mark -
#pragma mark Array Setup

- (void)setupAnimalsArray
{
    NSArray *animals = @[
    [Animal itemWithTitle:@"White Dog Portrait" withImage:[UIImage imageNamed:@"ID-10034505.jpg"] withCreator:@"photostock"],
    [Animal itemWithTitle:@"Black Labrador Retriever" withImage:[UIImage imageNamed:@"ID-1009881.jpg"] withCreator:@"Michal Marcol"],
    [Animal itemWithTitle:@"Anxious Dog" withImage:[UIImage imageNamed:@"ID-100120.jpg"] withCreator:@"James Barker"],
    [Animal itemWithTitle:@"Husky Dog" withImage:[UIImage imageNamed:@"ID-100136.jpg"] withCreator:@"James Barker"],
    [Animal itemWithTitle:@"Puppy" withImage:[UIImage imageNamed:@"ID-100140.jpg"] withCreator:@"James Barker"],
    [Animal itemWithTitle:@"Black Labrador Puppy" withImage:[UIImage imageNamed:@"ID-10018395.jpg"] withCreator:@"James Barker"],
    [Animal itemWithTitle:@"Yellow Labrador" withImage:[UIImage imageNamed:@"ID-10016005.jpg"] withCreator:@"m_bartosch"],
    [Animal itemWithTitle:@"Black Labrador" withImage:[UIImage imageNamed:@"ID-10012923.jpg"] withCreator:@"Felixco, Inc."],
    [Animal itemWithTitle:@"Sleepy Dog" withImage:[UIImage imageNamed:@"ID-10021769.jpg"] withCreator:@"Maggie Smith"],
    [Animal itemWithTitle:@"English Springer Spaniel Puppy" withImage:[UIImage imageNamed:@"ID-10056667.jpg"] withCreator:@"Tina Phillips"],
    [Animal itemWithTitle:@"Intelligent Dog" withImage:[UIImage imageNamed:@"ID-100137.jpg"] withCreator:@"James Barker"]
    ];
    
    self.arrayOfAnimals = [NSMutableArray arrayWithArray:animals];
    
    [self.myTableView reloadData];
}

#pragma mark -
#pragma mark UITableView Datasource/Delegate

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [_arrayOfAnimals count];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 54;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *cellMainNibID = @"cellMain";
    
    _cellMain = [tableView dequeueReusableCellWithIdentifier:cellMainNibID];
    if (_cellMain == nil) {
        [[NSBundle mainBundle] loadNibNamed:@"MainCellRight" owner:self options:nil];
    }
    
    UIImageView *mainImage = (UIImageView *)[_cellMain viewWithTag:1];
    
    UILabel *imageTitle = (UILabel *)[_cellMain viewWithTag:2];
    UILabel *creator = (UILabel *)[_cellMain viewWithTag:3];
    
    if ([_arrayOfAnimals count] > 0)
    {
        Animal *currentRecord = [self.arrayOfAnimals objectAtIndex:indexPath.row];
        
        mainImage.image = currentRecord.image;
        imageTitle.text = [NSString stringWithFormat:@"%@", currentRecord.title];
        creator.text = [NSString stringWithFormat:@"%@", currentRecord.creator];
    }
    
    return _cellMain;
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

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    Animal *currentRecord = [self.arrayOfAnimals objectAtIndex:indexPath.row];
    
    // Return Data to delegate: either way is fine, although passing back the object may be more efficient
    // [_delegate imageSelected:currentRecord.image withTitle:currentRecord.title withCreator:currentRecord.creator];
    // [_delegate animalSelected:currentRecord];
    
    [_delegate animalSelected:currentRecord];
}

#pragma mark -
#pragma mark Default System Code

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self)
    {
    }
    return self;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}

@end
