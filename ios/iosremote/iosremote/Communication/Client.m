// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import "Client.h"
#import "Server.h"
#import "Receiver.h"
#import "CommunicationManager.h"

@interface Client() <NSStreamDelegate>

@property (nonatomic, strong) NSInputStream* mInputStream;
@property (nonatomic, strong) NSOutputStream* mOutputStream;

@property (nonatomic, strong) NSString* mPin;
@property (nonatomic, strong) NSString* mName;
@property uint mPort;

@property (nonatomic, weak) Server* mServer;
@property (nonatomic, weak) Receiver* mReceiver;
@property (nonatomic, weak) CommunicationManager* mComManager;

@property (nonatomic, retain) NSMutableData* mData;

@property BOOL mReady;

@end



@implementation Client

@synthesize mInputStream = _mInputStream;
@synthesize mOutputStream = _mOutputStream;
@synthesize mPin = _mPin;
@synthesize mName = _mName;
@synthesize mServer = _mServer;
@synthesize mComManager = _mComManager;
@synthesize mData = _mData;
@synthesize mReady = _mReady;

NSString * const CHARSET = @"UTF-8";

- (id) initWithServer:(Server*)server
            managedBy:(CommunicationManager*)manager
        interpretedBy:(Receiver*)receiver
{
    self.mPin = [self getPin];
    NSLog(@"mPin: %@", self.mPin);
    self.mName = [[UIDevice currentDevice] name];
    self.mServer = server;
    self.mComManager = manager;
    self.mReceiver = receiver;
    // hardcoded here to test the communication TODO
    self.mPort = 1599;
    
    return self;
}

- (NSString *) getPin
{
    
    NSString* newPin = [NSString stringWithFormat:@"%04d", arc4random() % 9999];
}

- (void)streamOpenWithIp:(NSString *)ip withPortNumber:(uint)portNumber
{
    NSLog(@"Connecting to %@:%u", ip, portNumber);
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault, (__bridge CFStringRef)ip, portNumber, &readStream, &writeStream);
    
    if(readStream && writeStream)
    {
        CFReadStreamSetProperty(readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        CFWriteStreamSetProperty(writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        
        //Setup mInputStream
        self.mInputStream = (__bridge NSInputStream *)readStream;
        [self.mInputStream setDelegate:self];
        [self.mInputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [self.mInputStream open];
        
        //Setup outputstream
        self.mOutputStream = (__bridge NSOutputStream *)writeStream;
        [self.mOutputStream setDelegate:self];
        [self.mOutputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [self.mOutputStream open];
    
//        NSLog(@"Stream opened %@ %@", @"iPad", self.mPin);
        
        NSArray *temp = [[NSArray alloc]initWithObjects:@"LO_SERVER_CLIENT_PAIR\n", self.mName, @"\n", self.mPin, @"\n\n", nil];
        
        NSString *command = [temp componentsJoinedByString:@""];
        
        [self sendCommand:command];
    }
}

- (void) sendCommand:(NSString *)aCommand
{
    NSLog(@"Sending command %@", aCommand);
    // UTF-8 as speficied in specification
    NSData * data = [aCommand dataUsingEncoding:NSUTF8StringEncoding];
    
    [self.mOutputStream write:(uint8_t *)[data bytes] maxLength:[data length]];
}

- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode {
    
    switch(eventCode) {
        case NSStreamEventOpenCompleted:
            NSLog(@"Connection established");
            break;
        case NSStreamEventErrorOccurred:
            NSLog(@"Connection error occured");
            break;
        case NSStreamEventHasBytesAvailable:
        {
            NSLog(@"NSStreamEventHasBytesAvailable");
            if(!self.mData) {
                self.mData = [NSMutableData data];
            }
            uint8_t buf[1024];
            unsigned int len = 0;
            len = [(NSInputStream *)stream read:buf maxLength:1024];
            if(len) {
                [self.mData appendBytes:(const void *)buf length:len];
                int bytesRead = 0;
                // bytesRead is an instance variable of type NSNumber.
                bytesRead += len;
            } else {
                NSLog(@"No data but received event for whatever reasons!");
            }
            
            NSString *str = [[NSString alloc] initWithData:self.mData
                                                  encoding:NSUTF8StringEncoding];
            NSLog(@"Data Received: %@", str);
            
            self.mData = nil;
        } break;
        default:
        {
        
        }
    
    }
}


- (void) connect
{
    [self streamOpenWithIp:self.mServer.serverAddress withPortNumber:self.mPort];
}



@end
